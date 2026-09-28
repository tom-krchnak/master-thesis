#include "salasym/exec.hpp"

#include <stdexcept>
#include <utility>


namespace sala::sym
{

void Executor::run()
{
    _run_completed = false;
    _states.reset();
    _records.clear();
    _searcher = make_searcher( _config.search );

    init();

    while ( !_states.empty() )
    {
        ExecState &curr = _searcher->select();
        std::array removed = { &curr };
        _searcher->update( nullptr, {}, removed );

        apply_outcome( curr, step( curr ) );
    }

    _run_completed = true;
}

void Executor::apply_outcome( ExecState &state, StepOutcome outcome )
{
    std::visit( [&]( auto &&effect ) {
        apply( state, std::move( effect ) );
    }, std::move( outcome ) );
}

void Executor::apply( ExecState &state, Advance )
{
    ++state.frames.back().loc.instr;
    publish_one( state );
}

void Executor::apply( ExecState &state, Return )
{
    if ( state.frames.size() == 1 )
    {
        finish_path( state, Stop{ StopKind::Return } );
        return;
    }

    state.frames.pop_back();
    publish_one( state );
}

void Executor::apply( ExecState &state, Jump jump )
{
    auto &location = state.frames.back().loc;
    location.block = jump.block;
    location.instr = 0;
    publish_one( state );
}

void Executor::apply( ExecState &state, Call &&call )
{
    const auto &function = _program.functions()[ call.callee ];
    if ( function.is_external() )
    {
        exec_external_call( state, call );
        apply( state, Advance{} );
        return;
    }

    const ProgramLocation site = state.frames.back().loc;
    Frame callee = make_frame( state, call.callee );
    callee.call_site = site;

    for ( std::size_t i = 0; i < call.args.size(); ++i )
        state.memory.store( callee.param( i ), std::move( call.args[ i ] ) );

    ++state.frames.back().loc.instr;
    state.frames.push_back( std::move( callee ) );
    publish_one( state );
}

void Executor::apply( ExecState &state, Split &&split )
{
    std::array successors = { &_states.fork( state ), &state };
    for ( std::size_t i = 0; i < successors.size(); ++i )
    {
        auto &successor = *successors[ i ];
        auto &continuation = split.alts[ split.alts.size() - 1 - i ];
        successor.path.add( std::move( continuation.cond ) );
        auto &location = successor.frames.back().loc;
        location.block = continuation.block;
        location.instr = 0;
    }

    _searcher->update( nullptr, successors, {} );
}

void Executor::apply( ExecState &state, Stop stop )
{
    finish_path( state, stop );
}

void Executor::publish_one( ExecState &state )
{
    std::array added = { &state };
    _searcher->update( nullptr, added, {} );
}

StepOutcome Executor::step( ExecState &state ) const
{
    const auto &frame = state.frames.back();

    const auto &funct = _program.functions()[ frame.loc.funct ];
    const auto &block = funct.basic_blocks()[ frame.loc.block ];
    const auto &instr = block.instructions()[ frame.loc.instr ];

    switch ( instr.opcode() )
    {
        case Opcode::NOP:
            if ( instr.modifier() != Modifier::NONE )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            return Advance{};

        case Opcode::HALT:
            if ( instr.modifier() != Modifier::NONE )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            return Stop{ StopKind::Halt };

        case Opcode::COPY:
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        // case Opcode::DIV:
        // case Opcode::REM:
        // case Opcode::AND:
        // case Opcode::OR:
        // case Opcode::XOR:
        // case Opcode::SHL:
        // case Opcode::SHR:
        case Opcode::EXTEND:
        // case Opcode::TRUNCATE:
        // case Opcode::P2I:
        // case Opcode::I2P:
        case Opcode::LESS:
        // case Opcode::LESS_EQUAL:
        case Opcode::GREATER:
        // case Opcode::GREATER_EQUAL:
        case Opcode::EQUAL:
        // case Opcode::UNEQUAL:
            return exec_integer( state, instr );

        case Opcode::JUMP:
        case Opcode::BRANCH:
        case Opcode::CALL:
        case Opcode::RET:
            return exec_control( state, instr );

        case Opcode::ADDRESS:
        // case Opcode::LOAD: // TODO1
        case Opcode::STORE:
        // case Opcode::MEMCPY:
        // case Opcode::MEMMOVE:
        // case Opcode::MEMSET:
        // case Opcode::MOVEPTR: // TODO1
        // case Opcode::ALLOCA:
        // case Opcode::STACKSAVE:
        // case Opcode::STACKRESTORE:
        // case Opcode::MALLOC: // TODO2
        // case Opcode::FREE: // TODO2
            return exec_memory( state, instr );


        default:
            return Stop{ StopKind::Unsupported, UnsupportedReason::Opcode };
    }
}

static u64 read_bytes_le( const std::vector< std::uint8_t > &bytes )
{
    u64 value = 0;
    for ( std::size_t i = 0; i < bytes.size(); ++i )
        value |= u64( bytes[ i ] ) << ( 8 * i );

    return value;
}

StepOutcome Executor::exec_integer( ExecState &state,
    const sala::Instruction &instr ) const
{
    const auto &frame = state.frames.back();

    const auto &ops = instr.operands();
    const auto &descriptors = instr.descriptors();

    switch ( instr.opcode() )
    {
        case Opcode::COPY:
        {
            if ( instr.modifier() != Modifier::NONE )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || ( descriptors[ 1 ] != Descriptor::LOCAL
                        && descriptors[ 1 ] != Descriptor::CONSTANT ) )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            Pointer dest = frame.local( ops[ 0 ] );
            Expr value = descriptors[ 1 ] == Descriptor::LOCAL
                ? state.memory.load( frame.local( ops[ 1 ] ) )
                : Expr::constant( read_bytes_le( _program.constants()[ ops[ 1 ] ].bytes() ),
                    state.memory.load( dest ).width() );

            state.memory.store( dest, std::move( value ) );
            return Advance{};
        }

        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        {
            if ( instr.modifier() != Modifier::SIGNED
                    && instr.modifier() != Modifier::UNSIGNED )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || descriptors[ 1 ] != Descriptor::LOCAL
                    || descriptors[ 2 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = instr.opcode() == Opcode::ADD ? Expr::add( lhs, rhs )
                : instr.opcode() == Opcode::SUB ? Expr::sub( lhs, rhs )
                : Expr::mul( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            return Advance{};
        }

        case Opcode::LESS:
        case Opcode::GREATER:
        {
            if ( instr.modifier() != Modifier::SIGNED )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || descriptors[ 1 ] != Descriptor::LOCAL
                    || ( descriptors[ 2 ] != Descriptor::LOCAL
                        && descriptors[ 2 ] != Descriptor::CONSTANT ) )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr rhs = descriptors[ 2 ] == Descriptor::LOCAL
                ? state.memory.load( frame.local( ops[ 2 ] ) )
                : Expr::constant(
                    read_bytes_le( _program.constants()[ ops[ 2 ] ].bytes() ), lhs.width() );
            Expr result = instr.opcode() == Opcode::LESS
                ? Expr::slt( lhs, rhs )
                : Expr::sgt( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            return Advance{};
        }

        case Opcode::EQUAL:
        {
            if ( instr.modifier() != Modifier::UNSIGNED )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || descriptors[ 1 ] != Descriptor::LOCAL
                    || descriptors[ 2 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::eq( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            return Advance{};
        }

        case Opcode::EXTEND:
        {
            if ( instr.modifier() != Modifier::UNSIGNED )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || descriptors[ 1 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            Pointer dest = frame.local( ops[ 0 ] );
            u32 target_width = state.memory.load( dest ).width();

            const Expr &src = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr result = Expr::zext( src, target_width );

            state.memory.store( dest, std::move( result ) );
            return Advance{};
        }

        default:
            throw std::logic_error( "unexpected integer opcode" );
    }
}

StepOutcome Executor::exec_control( const ExecState &state,
    const sala::Instruction &instr ) const
{
    if ( instr.modifier() != Modifier::NONE )
        return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };

    const auto &frame = state.frames.back();
    const auto &funct = _program.functions()[ frame.loc.funct ];
    const auto &block = funct.basic_blocks()[ frame.loc.block ];

    switch ( instr.opcode() )
    {
        case Opcode::JUMP:
            return Jump{ block.successors().front() };

        case Opcode::BRANCH:
        {
            if ( instr.descriptors()[ 0 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            const Expr cond = state.memory.load( frame.local( instr.operands()[ 0 ] ) );
            return Split{ std::array{
                GuardedContinuation{ cond, block.successors()[ 1 ] },
                GuardedContinuation{ Expr::logical_not( cond ), block.successors()[ 0 ] },
            } };
        }

        case Opcode::CALL:
        {
            const auto &ops = instr.operands();
            const auto &descriptors = instr.descriptors();
            if ( descriptors[ 0 ] != Descriptor::FUNCTION )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            u32 target = ops[ 0 ];
            const auto &function = _program.functions()[ target ];
            if ( function.is_external() )
            {
                if ( function.name() != "__VERIFIER_nondet_int" )
                    return Stop{ StopKind::Unsupported, UnsupportedReason::ExternalModel };
                if ( ops.size() != 2 )
                    return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };
            }
            else if ( ops.size() - 1 != function.parameters().size() )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            for ( std::size_t i = 1; i < descriptors.size(); ++i )
            {
                if ( descriptors[ i ] != Descriptor::LOCAL
                        && descriptors[ i ] != Descriptor::PARAMETER )
                    return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };
            }

            return Call{ target, capture_call_args( state, instr ) };
        }

        case Opcode::RET:
            return Return{};

        default:
            throw std::logic_error( "unexpected control opcode" );
    }
}

void Executor::exec_external_call( ExecState &state, const Call &call )
{
    const auto &function = _program.functions()[ call.callee ];
    if ( function.name() != "__VERIFIER_nondet_int" || call.args.size() != 1 )
        throw std::logic_error( "unsupported call passed to external model" );

    Pointer dest = call.args.front().as_pointer();
    u32 width = state.memory.load( dest ).width();
    Expr sym = Expr::symbol( "sym" + std::to_string( _next_symbol_id++ ), width );
    state.memory.store( dest, std::move( sym ) );
}

StepOutcome Executor::exec_memory( ExecState &state,
    const sala::Instruction &instr ) const
{
    const auto &frame = state.frames.back();

    if ( instr.modifier() != Modifier::NONE )
        return Stop{ StopKind::Unsupported, UnsupportedReason::Modifier };

    const auto &ops = instr.operands();
    const auto &descriptors = instr.descriptors();

    switch ( instr.opcode() )
    {
        case Opcode::ADDRESS:
        {
            if ( descriptors[ 0 ] != Descriptor::LOCAL
                    || descriptors[ 1 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            Pointer target = frame.local( ops[ 1 ] );
            Expr address = Expr::address( target.obj, target.offset );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( address ) );
            return Advance{};
        }

        case Opcode::STORE:
        {
            if ( descriptors[ 0 ] != Descriptor::PARAMETER
                    || descriptors[ 1 ] != Descriptor::LOCAL )
                return Stop{ StopKind::Unsupported, UnsupportedReason::Operand };

            const Expr &ptr_value = state.memory.load( frame.param( ops[ 0 ] ) );
            const Expr &value = state.memory.load( frame.local( ops[ 1 ] ) );

            // TODO: ptr_value can be symbolic, fork/ite expr
            state.memory.store( ptr_value.as_pointer(), value );
            return Advance{};
        }

        default:
            throw std::logic_error( "unexpected memory opcode" );
    }
}


ObjId Executor::alloc( ExecState &state, u32 width )
{
    ObjId id = _next_obj_id++;
    state.memory.create( id, width );
    return id;
}

Frame Executor::make_frame( ExecState &state, u32 func_index )
{
    const auto &function = _program.functions()[ func_index ];

    Frame frame;
    frame.loc = { func_index, 0, 0 };

    for ( const auto &param : function.parameters() )
        frame.push_param( alloc( state, u32( param.num_bytes() * 8 ) ) );

    for ( const auto &local : function.local_variables() )
        frame.push_local( alloc( state, u32( local.num_bytes() * 8 ) ) );

    return frame;
}

void Executor::init()
{
    ExecState &initial = _states.create();
    initial.frames.push_back( make_frame( initial, _program.entry_function() ) );

    // TODO: remove hardcoded result size
    ObjId result = alloc( initial, 32 );
    initial.memory.store( initial.frames.back().param( 0 ),
        Expr::address( result, 0 ) );

    publish_one( initial );
}



void Executor::finish_path( ExecState &state, Stop stop )
{
    ExecutionRecord record{ stop, state.frames.back().loc, {} };

    for ( const Frame &frame : state.frames )
    {
        if ( frame.call_site )
            record.call_stack.push_back( *frame.call_site );
    }

    record.call_stack.push_back( record.location );

    _records.push_back( std::move( record ) );
    _states.complete( &state );
}


std::vector< Expr > Executor::capture_call_args(
    const ExecState &state, const sala::Instruction &instr )
{
    const auto &caller = state.frames.back();
    const auto &operands = instr.operands();
    const auto &descriptors = instr.descriptors();

    std::vector< Expr > args;
    args.reserve( operands.size() - 1 );

    for ( std::size_t i = 1; i < operands.size(); ++i )
    {
        Pointer ptr = [ &, i ]()
        {
            switch ( descriptors[ i ] )
            {
                case Descriptor::LOCAL:
                    return caller.local( operands[ i ] );
                case Descriptor::PARAMETER:
                    return caller.param( operands[ i ] );
                default:
                    throw std::logic_error( "unsupported call source passed to capture_call_args" );
            }
        }();

        args.push_back( state.memory.load( ptr ) );
    }

    return args;
}

} // namespace sala::sym