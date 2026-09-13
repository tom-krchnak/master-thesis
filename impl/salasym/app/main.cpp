#include "sala/program.hpp"
#include "sala/streaming.hpp"

#include "salasym/exec.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main( int argc, char **argv )
{
    if ( argc < 2 )
    {
        std::cerr << "usage: " << argv[ 0 ] << " INPUT_FILE" << "\n";
        return 1;
    }

    std::string path = argv[ 1 ];
    std::ifstream in( path );
    if ( !in )
    {
        std::cerr << "failed to open " << path << "\n";
        return 1;
    }

    sala::Program program;
    in >> program;

    sala::sym::ExecConfig config;
    sala::sym::Executor executor( program, config );

    sala::sym::ExecState state = executor.make_initial_state( program.entry_function() );

    // TODO: remove harcoded result size
    sala::sym::ObjId result = executor.alloc( state, 32 );
    state.memory.store( state.frames.back().param( 0 ),
        sala::sym::Expr::address( result, 0 ) );

    executor.run( state );

    std::cout << "result = " << state.memory.load( { result, 0 } ).to_string() << "\n";
}
