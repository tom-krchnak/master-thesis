#pragma once

#include "sala/program.hpp"

#include "salasym/config.hpp"
#include "salasym/search.hpp"
#include "salasym/state_store.hpp"
#include "salasym/types.hpp"

#include <span>


namespace sala::sym
{

struct Executor
{
    Executor( const sala::Program &program,
        const ExecConfig &config )
        : _program( program )
        , _config( config )
    {}

    ObjId run();

    std::span< ExecState * const > completed_states() const
    {
        return _states.completed();
    }

private:

    const sala::Program &_program;
    const ExecConfig &_config;

    ptr< ISearcher > _searcher;
    StateStore _states;
    u32 _next_obj_id = 0;
    u32 _next_symbol_id = 0;

    void exec_integer( ExecState &state, const sala::Instruction &instr );
    bool exec_control( ExecState &state, const sala::Instruction &instr );
    void exec_memory( ExecState &state, const sala::Instruction &instr );

    void exec_external_call( ExecState &state, const sala::Instruction &instr, u32 target );

    Frame make_frame( ExecState &state, u32 func_index );

    void terminate( ExecState &state );

    void exec( ExecState &state );
    ObjId init();

    ObjId alloc( ExecState &state, u32 width );

};

} // namespace sala::sym