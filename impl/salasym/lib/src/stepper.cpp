#include "salasym/stepper.hpp"
#include "utility/invariants.hpp"

#include <cstddef>
#include <utility>

namespace sala::sym
{

StepOutcome InstructionStepper::step( ExecState &state ) const
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

StepOutcome InstructionStepper::exec_integer( ExecState &state,
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
            UNREACHABLE();
    }
}

StepOutcome InstructionStepper::exec_control( const ExecState &state,
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
            if ( !function.is_external()
                    && ops.size() - 1 != function.parameters().size() )
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
            UNREACHABLE();
    }
}

StepOutcome InstructionStepper::exec_memory( ExecState &state,
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
            UNREACHABLE();
    }
}

std::vector< Expr > InstructionStepper::capture_call_args(
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
                    UNREACHABLE();
            }
        }();

        args.push_back( state.memory.load( ptr ) );
    }

    return args;
}

} // namespace sala::sym
