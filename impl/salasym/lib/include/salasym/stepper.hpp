#pragma once

#include "sala/program.hpp"

#include "salasym/outcome.hpp"
#include "salasym/state.hpp"

#include <vector>

namespace sala::sym
{

struct InstructionStepper
{
    explicit InstructionStepper( const sala::Program &program )
        : _program( program )
    {}

    StepOutcome step( ExecState &state ) const;

private:

    const sala::Program &_program;

    StepOutcome exec_integer( ExecState &state, const sala::Instruction &instr ) const;
    StepOutcome exec_control( const ExecState &state, const sala::Instruction &instr ) const;
    StepOutcome exec_memory( ExecState &state, const sala::Instruction &instr ) const;

    static std::vector< Expr > capture_call_args(
        const ExecState &state, const sala::Instruction &instr );
};

} // namespace sala::sym
