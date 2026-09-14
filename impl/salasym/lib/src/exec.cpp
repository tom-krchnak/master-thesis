#include "salasym/exec.hpp"


namespace sala::sym
{

ObjId Executor::run()
{
    _states.reset();
    _searcher = make_searcher( _config.search );

    ObjId result = init();

    while ( !_states.empty() )
    {
        ExecState &curr = _searcher->select();

        exec( curr );
    }

    return result;
}

void Executor::exec( ExecState &state )
{
    auto &frame = state.frames.back();

    const auto &funct = _program.functions()[ frame.funct ];
    const auto &block = funct.basic_blocks()[ frame.block ];
    const auto &instr = block.instructions()[ frame.instr ];

    using Opcode = sala::Instruction::Opcode;

    bool advance = true;

    switch ( instr.opcode() )
    {
        case Opcode::NOP:
            break;

        case Opcode::HALT:
            terminate( state );
            advance = false;
            break;

        case Opcode::COPY:
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        // case Opcode::DIV:
        // case Opcode::REM:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::SHL:
        case Opcode::SHR:
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
            exec_integer( state, instr );
            break;

        case Opcode::JUMP:
        case Opcode::BRANCH:
        case Opcode::CALL:
        case Opcode::RET:
            advance = exec_control( state, instr );
            break;

        case Opcode::ADDRESS:
        // case Opcode::LOAD:
        case Opcode::STORE:
        // case Opcode::MEMCPY:
        // case Opcode::MEMMOVE:
        // case Opcode::MEMSET:
        // case Opcode::MOVEPTR:
        // case Opcode::ALLOCA:
        // case Opcode::STACKSAVE:
        // case Opcode::STACKRESTORE:
        // case Opcode::MALLOC:
        // case Opcode::FREE:
            exec_memory( state, instr );
            break;

        default:
            UNREACHABLE();
    }

    if ( advance )
        ++frame.instr;
}

static u64 read_bytes_le( const std::vector< std::uint8_t > &bytes )
{
    u64 value = 0;
    for ( std::size_t i = 0; i < bytes.size(); ++i )
        value |= u64( bytes[ i ] ) << ( 8 * i );

    return value;
}

void Executor::exec_integer( ExecState &state,
    const sala::Instruction &instr )
{
    auto &frame = state.frames.back();

    using Opcode = sala::Instruction::Opcode;
    using Modifier = sala::Instruction::Modifier;
    using Descriptor = sala::Instruction::Descriptor;

    switch ( instr.opcode() )
    {
        case Opcode::COPY:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            Pointer dest = frame.local( ops[ 0 ] );
            Expr value = instr.descriptors()[ 1 ] == Descriptor::LOCAL
                ? state.memory.load( frame.local( ops[ 1 ] ) )
                : Expr::constant( read_bytes_le( _program.constants()[ ops[ 1 ] ].bytes() ),
                    state.memory.load( dest ).width() );

            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL
                || instr.descriptors()[ 1 ] == Descriptor::CONSTANT );
            state.memory.store( dest, std::move( value ) );
            break;
        }

        case Opcode::ADD:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::add( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::SUB:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::sub( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::MUL:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 2 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr result = Expr::mul( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::AND:
        {
            INVARIANT( instr.modifier() == Modifier::NONE );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::bit_and( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::OR:
        {
            INVARIANT( instr.modifier() == Modifier::NONE );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::bit_or( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::XOR:
        {
            INVARIANT( instr.modifier() == Modifier::NONE );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::bit_xor( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::SHL:
        {
            INVARIANT( instr.modifier() == Modifier::NONE );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::CONSTANT );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr rhs = Expr::constant(
                read_bytes_le( _program.constants()[ ops[ 2 ] ].bytes() ), lhs.width() );
            Expr result = Expr::shl( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::SHR:
        {
            INVARIANT( instr.modifier() == Modifier::UNSIGNED );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::CONSTANT );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr rhs = Expr::constant(
                read_bytes_le( _program.constants()[ ops[ 2 ] ].bytes() ), lhs.width() );
            Expr result = Expr::lshr( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::LESS:
        case Opcode::GREATER:
        {
            INVARIANT(
                ( instr.opcode() == Opcode::LESS
                    && instr.modifier() == Modifier::SIGNED )
                || ( instr.opcode() == Opcode::GREATER
                    && ( instr.modifier() == Modifier::SIGNED
                        || instr.modifier() == Modifier::UNSIGNED ) ) );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL
                || instr.descriptors()[ 2 ] == Descriptor::CONSTANT );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr rhs = instr.descriptors()[ 2 ] == Descriptor::LOCAL
                ? state.memory.load( frame.local( ops[ 2 ] ) )
                : Expr::constant(
                    read_bytes_le( _program.constants()[ ops[ 2 ] ].bytes() ), lhs.width() );
            Expr result = instr.opcode() == Opcode::LESS
                ? Expr::slt( lhs, rhs )
                : instr.modifier() == Modifier::SIGNED
                    ? Expr::sgt( lhs, rhs )
                    : Expr::ugt( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::EQUAL:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();
            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );
            const Expr &rhs = state.memory.load( frame.local( ops[ 2 ] ) );
            Expr result = Expr::eq( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), std::move( result ) );
            break;
        }

        case Opcode::EXTEND:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();

            Pointer dest = frame.local( ops[ 0 ] );
            u32 target_width = state.memory.load( dest ).width();

            const Expr &src = state.memory.load( frame.local( ops[ 1 ] ) );
            Expr result = Expr::zext( src, target_width );

            state.memory.store( dest, result );
            break;
        }

        default:
            UNREACHABLE();
    }
}

bool Executor::exec_control( ExecState &state,
    const sala::Instruction &instr )
{
    using Opcode = sala::Instruction::Opcode;
    using Descriptor = sala::Instruction::Descriptor;

    auto &frame = state.frames.back();
    const auto &funct = _program.functions()[ frame.funct ];
    const auto &block = funct.basic_blocks()[ frame.block ];

    switch ( instr.opcode() )
    {
        case Opcode::JUMP:
        {
            INVARIANT( block.successors().size() == 1 );
            frame.block = block.successors().front();
            frame.instr = 0;
            return false;
        }

        case Opcode::BRANCH:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( block.successors().size() == 2 );

            const Expr condition = state.memory.load( frame.local( instr.operands()[ 0 ] ) );
            ExecState &forked = _states.fork( state );

            state.path.add( Expr::logical_not( condition ) );
            forked.path.add( condition );

            frame.block = block.successors()[ 0 ];
            frame.instr = 0;
            forked.frames.back().block = block.successors()[ 1 ];
            forked.frames.back().instr = 0;

            std::array added = { &forked };
            _searcher->update( &state, added, {} );
            return false;
        }

        case Opcode::CALL:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::FUNCTION );

            const auto &ops = instr.operands();
            u32 target = ops[ 0 ];

            if ( _program.functions()[ target ].is_external() )
            {
                exec_external_call( state, instr, target );
                return true;
            }

            Frame callee = make_frame( state, target );
            const auto &function = _program.functions()[ target ];

            auto &caller = state.frames.back();

            for ( std::size_t i = 0; i < function.parameters().size(); ++i )
            {
                INVARIANT( instr.descriptors()[ i + 1 ] == Descriptor::LOCAL );

                Expr value = state.memory.load( caller.local( ops[ i + 1 ] ) );
                state.memory.store( callee.param( u32( i ) ), value );
            }

            state.frames.push_back( std::move( callee ) );
            return false;
        }

        case Opcode::RET:
        {
            state.frames.pop_back();

            if ( state.frames.empty() )
                terminate( state );

            return false;
        }

        default:
            UNREACHABLE();
    }
}

void Executor::exec_external_call( ExecState &state,
    const sala::Instruction &instr, u32 target )
{
    using Descriptor = sala::Instruction::Descriptor;

    const auto &function = _program.functions()[ target ];

    if ( function.name() == "__VERIFIER_nondet_int"
        || function.name() == "__VERIFIER_nondet_uint" )
    {
        auto &frame = state.frames.back();

        INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );

        const Expr &p0 = state.memory.load( frame.local( instr.operands()[ 1 ] ) );
        Pointer dest = p0.as_pointer();

        u32 width = state.memory.load( dest ).width();
        Expr sym = Expr::symbol( "sym" + std::to_string( _next_symbol_id++ ), width );

        state.memory.store( dest, sym );
        return;
    }

    UNREACHABLE();
}

void Executor::exec_memory( ExecState &state,
    const sala::Instruction &instr )
{
    auto &frame = state.frames.back();

    using Opcode = sala::Instruction::Opcode;
    using Descriptor = sala::Instruction::Descriptor;

    switch ( instr.opcode() )
    {
        case Opcode::ADDRESS:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();

            Pointer target = frame.local( ops[ 1 ] );
            Expr address = Expr::address( target.obj, target.offset );

            state.memory.store( frame.local( ops[ 0 ] ), address );
            break;
        }

        case Opcode::STORE:
        {
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::PARAMETER );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );

            const auto &ops = instr.operands();

            const Expr &ptr_value = state.memory.load( frame.param( ops[ 0 ] ) );
            const Expr &value = state.memory.load( frame.local( ops[ 1 ] ) );

            state.memory.store( ptr_value.as_pointer(), value );
            break;
        }

        default:
            UNREACHABLE();
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
    frame.funct = func_index;
    frame.block = 0;
    frame.instr = 0;

    for ( const auto &param : function.parameters() )
        frame.push_param( alloc( state, u32( param.num_bytes() * 8 ) ) );

    for ( const auto &local : function.local_variables() )
        frame.push_local( alloc( state, u32( local.num_bytes() * 8 ) ) );

    return frame;
}

ObjId Executor::init()
{
    ExecState &initial = _states.create();
    initial.frames.push_back( make_frame( initial, _program.entry_function() ) );

    // TODO: remove hardcoded result size
    ObjId result = alloc( initial, 32 );
    initial.memory.store( initial.frames.back().param( 0 ),
        Expr::address( result, 0 ) );

    std::array added = { &initial };
    _searcher->update( nullptr, added, {} );

    return result;
}

void Executor::terminate( ExecState &state )
{
    _states.complete( &state );

    std::array removed = { &state };
    _searcher->update( nullptr, {}, removed );
}

} // namespace sala::sym