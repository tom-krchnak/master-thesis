#pragma once

#include "salasym/state.hpp"
#include "utility/invariants.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace sala::sym
{

struct StateStore
{
    ExecState &create()
    {
        return adopt( make_ptr< ExecState >( next_id() ) );
    }

    ExecState &fork( const ExecState &source )
    {
        auto copy = make_ptr< ExecState >( source );
        copy->_id = next_id();
        return adopt( std::move( copy ) );
    }

    void erase( ExecState &state )
    {
        const std::size_t slot = state._store_slot;
        INVARIANT( slot < _owned.size() );
        INVARIANT( _owned[ slot ].get() == &state );

        const std::size_t last = _owned.size() - 1;
        if ( slot != last )
        {
            std::swap( _owned[ slot ], _owned[ last ] );
            _owned[ slot ]->_store_slot = slot;
        }

        _owned.pop_back();
    }

    void reset() noexcept
    {
        _owned.clear();
        _next_state_id = 0;
    }

    bool empty() const
    {
        return _owned.empty();
    }

private:

    ExecState &adopt( ptr< ExecState > state )
    {
        state->_store_slot = _owned.size();
        ExecState &result = *state;
        _owned.push_back( std::move( state ) );
        return result;
    }


    StateId next_id()
    {
        return StateId( _next_state_id++ );
    }

    std::vector< ptr< ExecState > > _owned;

    u32 _next_state_id = 0;
};

} // namespace sala::sym
