#pragma once

#include "sala/program.hpp"

#include "salasym/config.hpp"
#include "salasym/outcome.hpp"
#include "salasym/search.hpp"
#include "salasym/state_store.hpp"
#include "salasym/stepper.hpp"
#include "salasym/types.hpp"

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

    ptr< ISearcher > _searcher;
    StateStore _states;

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

    void publish_one( ExecState &state );
    void init();

    ObjId alloc( ExecState &state, u32 width );

};

} // namespace sala::sym