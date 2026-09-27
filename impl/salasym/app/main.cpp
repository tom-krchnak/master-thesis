#include "sala/program.hpp"
#include "sala/streaming.hpp"

#include "salasym/exec.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

struct options
{
    std::string input;
};

static options parse_options( int argc, char **argv )
{
    options opts;

    if ( argc < 2 )
        throw std::runtime_error( "missing input file" );

    opts.input = argv[ 1 ];

    return opts;
}

static void salasym( const options &opts )
{
    std::ifstream in( opts.input );
    if ( !in )
        throw std::runtime_error( "failed to open " + opts.input );

    sala::Program program;
    in >> program;

    sala::sym::ExecConfig config;
    sala::sym::Executor executor( program, config );

    executor.run();

    for ( const auto &record : executor.records() )
    {
        const auto &loc = record.location;

        std::cout << "candidate path: "
            << to_str( record.stop.kind ) << " at "
            << loc.funct << ":" << loc.block << ":" << loc.instr
            << "\n";
    }
}

int main( int argc, char **argv )
{
    try
    {
        options opts = parse_options( argc, argv );
        salasym( opts );

        return 0;
    }
    catch ( std::exception &e )
    {
        std::cerr << "error: " << e.what() << "\n";
    }
    catch ( ... )
    {
        std::cerr << "unknown error\n";
    }

    return 1;
}
