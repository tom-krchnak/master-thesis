#pragma once

#include "sala/program.hpp"

#include "salasym/config.hpp"
#include "salasym/search.hpp"
#include "salasym/state_store.hpp"
#include "salasym/types.hpp"

#include <array>
#include <span>
#include <variant>


namespace sala::sym
{

enum class StopKind
{
    Return,
    Halt,
    Unsupported,
};

enum class UnsupportedReason
{
    None,
    Opcode,
    Modifier,
    Operand,
    ExternalModel,
};

struct Stop
{
    StopKind kind;
    UnsupportedReason reason = UnsupportedReason::None;
};

struct ExecutionRecord
{
    Stop stop;
    ProgramLocation location;
    std::vector< ProgramLocation > call_stack;
};

struct Advance {};
struct Return {};

struct Jump
{
    u32 block;
};

struct Call
{
    u32 callee;
    std::vector< Expr > args;
};

struct GuardedContinuation
{
    Expr cond;
    u32 block;
};

struct Split
{
    std::array< GuardedContinuation, 2 > alts;
};

using StepOutcome = std::variant< Advance, Return, Jump, Call, Split, Stop >;


struct Executor
{
    Executor( const sala::Program &program,
        const ExecConfig &config )
        : _program( program )
        , _config( config )
    {}

    void run();

    std::span< const ExecutionRecord > records() const
    {
        INVARIANT( _run_completed );
        return _records;
    }

private:

    const sala::Program &_program;
    const ExecConfig &_config;

    ptr< ISearcher > _searcher;
    StateStore _states;

    u32 _next_obj_id = 0;
    u32 _next_symbol_id = 0;

    std::vector< ExecutionRecord > _records;
    bool _run_completed = false;

    StepOutcome exec_integer( ExecState &state, const sala::Instruction &instr ) const;
    StepOutcome exec_control( const ExecState &state, const sala::Instruction &instr ) const;
    StepOutcome exec_memory( ExecState &state, const sala::Instruction &instr ) const;

    void exec_external_call( ExecState &state, const sala::Instruction &instr, u32 target );

    Frame make_frame( ExecState &state, u32 func_index );

    void terminate( ExecState &state );
    void finish_path( ExecState &state, Stop stop );

    StepOutcome step( ExecState &state ) const;
    ObjId init();

    ObjId alloc( ExecState &state, u32 width );

    static std::vector< Expr > capture_call_args(
        const ExecState &state, const sala::Instruction &instr );

};

inline std::string_view to_str( StopKind kind )
{
    switch ( kind )
    {
        case StopKind::Return:      return "return";
        case StopKind::Halt:        return "halt";
        case StopKind::Unsupported: return "unsupported";
    }

    UNREACHABLE();
}

} // namespace sala::sym