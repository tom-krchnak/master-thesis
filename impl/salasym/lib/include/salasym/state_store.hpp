#pragma once

#include "salasym/state.hpp"

#include <set>
#include <span>
#include <vector>

namespace sala::sym
{

struct StateStore
{
    ExecState &create()
    {
        return add( ExecState{ next_id() } );
    }

    ExecState &fork( const ExecState &source )
    {
        ExecState copy = source;
        copy.id = next_id();

        return add( std::move( copy ) );
    }

    void complete( ExecState *state )
    {
        INVARIANT( _active.erase( state ) == 1 );
        _completed.push_back( state );
    }

    bool contains( ExecState *state ) const
    {
        return _active.contains( state );
    }

    void reset()
    {
        _owned.clear();
        _active.clear();
        _completed.clear();

        _next_state_id = 0;
    }

    std::span< ExecState * const > completed() const
    {
        return _completed;
    }

    bool empty() const
    {
        return _active.empty();
    }

private:

    ExecState &add( ExecState state )
    {
        _owned.emplace_back( make_ptr< ExecState >( std::move( state ) ) );
        auto *result = _owned.back().get();

        auto [ _, inserted ] = _active.insert( result );
        INVARIANT( inserted );

        return *result;
    }


    StateId next_id()
    {
        return StateId( _next_state_id++ );
    }

    std::vector< ptr< ExecState > > _owned;
    std::set< ExecState *, ExecStateIDCmp > _active;
    std::vector< ExecState * > _completed;

    u32 _next_state_id = 0;
};

} // namespace sala::sym
