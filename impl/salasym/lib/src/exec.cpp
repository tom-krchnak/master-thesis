#include "salasym/exec.hpp"


namespace sala::sym
{

void Executor::run( ExecState &initial )
{
    _searcher = make_searcher( _config.search );

    _states.add( &initial );

    std::array added = { &initial };
    _searcher->update( nullptr, added, {} );

    while ( !_states.empty() )
    {
        ExecState &curr = _searcher->select();

        exec( curr );
    }
}

void Executor::exec( ExecState &state )
{
    auto &frame = state.frames.back();
    const auto &block = _program
        .functions()[ frame.funct ]
        .basic_blocks()[ frame.block ];
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

        // case Opcode::COPY:
        // case Opcode::ADD:
        // case Opcode::SUB:
        // case Opcode::MUL:
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
        // case Opcode::LESS:
        // case Opcode::LESS_EQUAL:
        case Opcode::GREATER:
        // case Opcode::GREATER_EQUAL:
        // case Opcode::EQUAL:
        // case Opcode::UNEQUAL:
            exec_integer( state, instr );
            break;

        // case Opcode::JUMP:
        // case Opcode::BRANCH:
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
        case Opcode::GREATER:
        {
            INVARIANT( instr.modifier() == Modifier::SIGNED );
            INVARIANT( instr.descriptors()[ 0 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 1 ] == Descriptor::LOCAL );
            INVARIANT( instr.descriptors()[ 2 ] == Descriptor::CONSTANT );

            const auto &ops = instr.operands();

            const Expr &lhs = state.memory.load( frame.local( ops[ 1 ] ) );

            const auto &constant = _program.constants()[ ops[ 2 ] ];
            Expr rhs = Expr::constant( read_bytes_le( constant.bytes() ), lhs.width() );

            Expr result = Expr::sgt( lhs, rhs );

            state.memory.store( frame.local( ops[ 0 ] ), result );
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

    switch ( instr.opcode() )
    {
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

    if ( function.name() == "__VERIFIER_nondet_int" )
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

ExecState Executor::make_initial_state( u32 func_index )
{
    ExecState state{ StateId( _next_state_id++ ) };
    state.frames.push_back( make_frame( state, func_index ) );
    return state;
}

void Executor::terminate( ExecState &state )
{
    _states.remove( &state );

    std::array removed = { &state };
    _searcher->update( nullptr, {}, removed );
}

} // namespace sala::sym