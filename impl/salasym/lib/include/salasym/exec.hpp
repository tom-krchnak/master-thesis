#pragma once

#include "sala/program.hpp"

#include "salasym/config.hpp"
#include "salasym/search.hpp"
#include "salasym/state_store.hpp"
#include "salasym/types.hpp"

#include <set>

namespace sala::sym
{

struct Executor
{
    Executor( const sala::Program &program,
        const ExecConfig &config )
        : _program( program )
        , _config( config )
    {}

    void run( ExecState &initial );

    void exec( ExecState &state );

    ExecState make_initial_state( u32 func_index );

    ObjId alloc( ExecState &state, u32 width );

private:

    const sala::Program &_program;
    const ExecConfig &_config;

    ptr< ISearcher > _searcher;
    StateStore _states;

    u32 _next_obj_id = 0;
    u32 _next_symbol_id = 0;
    u32 _next_state_id = 0;

    void exec_integer( ExecState &state, const sala::Instruction &instr );
    bool exec_control( ExecState &state, const sala::Instruction &instr );
    void exec_memory( ExecState &state, const sala::Instruction &instr );

    void exec_external_call( ExecState &state, const sala::Instruction &instr, u32 target );

    Frame make_frame( ExecState &state, u32 func_index );

    void terminate( ExecState &state );
};

} // namespace sala::sym