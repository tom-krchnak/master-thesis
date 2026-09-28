#include "salasym/exec.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <variant>


namespace sala::sym
{

void Executor::run()
{
    _run_completed = false;
    _states.reset();
    _records.clear();
    _searcher = make_searcher( _config.search );

    init();

    while ( !_searcher->empty() )
    {
        ExecState &curr = _searcher->take();

        apply_outcome( curr, _stepper.step( curr ) );
    }

    _run_completed = true;
}

void Executor::apply_outcome( ExecState &state, StepOutcome outcome )
{
    std::visit( [&]( auto &&effect ) {
        apply( state, std::move( effect ) );
    }, std::move( outcome ) );
}

void Executor::apply( ExecState &state, Advance )
{
    ++state.frames.back().loc.instr;
    publish_one( state );
}

void Executor::apply( ExecState &state, Return )
{
    if ( state.frames.size() == 1 )
    {
        finish_path( state, Stop{ StopKind::Return } );
        return;
    }

    state.frames.pop_back();
    publish_one( state );
}

void Executor::apply( ExecState &state, Jump jump )
{
    auto &location = state.frames.back().loc;
    location.block = jump.block;
    location.instr = 0;
    publish_one( state );
}

void Executor::apply( ExecState &state, Call &&call )
{
    const auto &function = _program.functions()[ call.callee ];
    if ( function.is_external() )
    {
        if ( function.name() != "__VERIFIER_nondet_int" )
        {
            apply( state, Stop{ StopKind::Unsupported, UnsupportedReason::ExternalModel } );
            return;
        }
        if ( call.args.size() != 1 )
        {
            apply( state, Stop{ StopKind::Unsupported, UnsupportedReason::Operand } );
            return;
        }

        Pointer dest = call.args.front().as_pointer();
        u32 width = state.memory.load( dest ).width();
        Expr sym = Expr::symbol( "sym" + std::to_string( _next_symbol_id++ ), width );
        state.memory.store( dest, std::move( sym ) );
        apply( state, Advance{} );
        return;
    }

    const ProgramLocation site = state.frames.back().loc;
    Frame callee = make_frame( state, call.callee );
    callee.call_site = site;

    for ( std::size_t i = 0; i < call.args.size(); ++i )
        state.memory.store( callee.param( i ), std::move( call.args[ i ] ) );

    ++state.frames.back().loc.instr;
    state.frames.push_back( std::move( callee ) );
    publish_one( state );
}

void Executor::apply( ExecState &state, Split &&split )
{
    std::array successors = { &state, &_states.fork( state ) };
    for ( std::size_t i = 0; i < successors.size(); ++i )
    {
        auto &successor = *successors[ i ];
        auto &continuation = split.alts[ i ];
        successor.path.add( std::move( continuation.cond ) );
        auto &location = successor.frames.back().loc;
        location.block = continuation.block;
        location.instr = 0;
    }

    _searcher->publish( successors );
}

void Executor::apply( ExecState &state, Stop stop )
{
    finish_path( state, stop );
}

void Executor::publish_one( ExecState &state )
{
    std::array ordered = { &state };
    _searcher->publish( ordered );
}


ObjId Executor::alloc( ExecState &state, u32 width )
{
    ObjId id = _next_obj_id++;
    state.memory.create( id, width );
    return id;
}

Frame Executor::make_frame( ExecState &state, u32 func_index )
{
    const auto &function = _program.functions()[ func_index ];

    Frame frame;
    frame.loc = { func_index, 0, 0 };

    for ( const auto &param : function.parameters() )
        frame.push_param( alloc( state, u32( param.num_bytes() * 8 ) ) );

    for ( const auto &local : function.local_variables() )
        frame.push_local( alloc( state, u32( local.num_bytes() * 8 ) ) );

    return frame;
}

void Executor::init()
{
    ExecState &initial = _states.create();
    initial.frames.push_back( make_frame( initial, _program.entry_function() ) );

    // TODO: remove hardcoded result size
    ObjId result = alloc( initial, 32 );
    initial.memory.store( initial.frames.back().param( 0 ),
        Expr::address( result, 0 ) );

    publish_one( initial );
}



void Executor::finish_path( ExecState &state, Stop stop )
{
    ExecutionRecord record{ stop, state.frames.back().loc, {} };

    for ( const Frame &frame : state.frames )
    {
        if ( frame.call_site )
            record.call_stack.push_back( *frame.call_site );
    }

    record.call_stack.push_back( record.location );

    _records.push_back( std::move( record ) );
    _states.erase( state );
}

} // namespace sala::sym