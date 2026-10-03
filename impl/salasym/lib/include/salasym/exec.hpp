#pragma once

#include "sala/program.hpp"

#include "salasym/config.hpp"
#include "salasym/outcome.hpp"
#include "salasym/search.hpp"
#include "salasym/state_store.hpp"
#include "salasym/stepper.hpp"
#include "salasym/types.hpp"
#include "utility/invariants.hpp"

#include <span>
#include <vector>


namespace sala::sym
{

struct ExecutionRecord
{
    Stop stop;
    ProgramLocation location;
    std::vector< ProgramLocation > call_stack;
};

struct Executor
{
    Executor( const sala::Program &program,
        const ExecConfig &config )
        : _program( program )
        , _config( config )
        , _stepper( program )
    {}

    ~Executor();

    void run();

    std::span< const ExecutionRecord > records() const
    {
        INVARIANT( _run_completed );
        return _records;
    }

private:

    const sala::Program &_program;
    const ExecConfig &_config;
    InstructionStepper _stepper;

    StateStore _states;
    ptr< ISearcher > _searcher;

    u32 _next_obj_id = 0;
    u32 _next_symbol_id = 0;

    std::vector< ExecutionRecord > _records;
    bool _run_completed = false;

    Frame make_frame( ExecState &state, u32 func_index );

    void finish_path( ExecState &state, Stop stop );

    void apply_outcome( ExecState &state, StepOutcome outcome );
    void apply( ExecState &state, Advance );
    void apply( ExecState &state, Return );
    void apply( ExecState &state, Jump jump );
    void apply( ExecState &state, Call &&call );
    void apply( ExecState &state, Split &&split );
    void apply( ExecState &state, Stop stop );

    void complete_split( ExecState &original,
        std::span< GuardedContinuation > alternatives,
        std::span< ExecState * > successors );

    void publish_one( ExecState &state );
    void clear_execution() noexcept;
    void init();

    ObjId alloc( ExecState &state, u32 width );

};

} // namespace sala::sym