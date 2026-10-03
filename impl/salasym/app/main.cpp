#include "sala/program.hpp"
#include "sala/streaming.hpp"

#include "salasym/exec.hpp"
#include "utility/invariants.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

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

static std::string_view unsupported_reason_name(
    sala::sym::UnsupportedReason reason )
{
    using sala::sym::UnsupportedReason;

    switch ( reason )
    {
        case UnsupportedReason::None:          return "none";
        case UnsupportedReason::Opcode:        return "opcode";
        case UnsupportedReason::Modifier:      return "modifier";
        case UnsupportedReason::Operand:       return "operand";
        case UnsupportedReason::ExternalModel: return "external model";
    }

    UNREACHABLE();
}

static void write_location( const sala::sym::ProgramLocation &location )
{
    std::cout << location.funct << ':' << location.block << ':'
              << location.instr;
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
        std::cout << "candidate path: "
            << to_str( record.stop.kind ) << " at ";
        write_location( record.location );

        if ( record.stop.kind == sala::sym::StopKind::Unsupported )
            std::cout << " (" << unsupported_reason_name( record.stop.reason )
                      << ')';

        if ( record.call_stack.size() > 1 )
        {
            std::cout << " call stack:";
            for ( const auto &location : record.call_stack )
            {
                std::cout << ' ';
                write_location( location );
            }
        }

        std::cout << '\n';
    }

    std::cout.flush();
    if ( !std::cout )
        throw std::runtime_error( "failed to write execution outcomes" );
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
