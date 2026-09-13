#pragma once

#include "salasym/state.hpp"

#include <set>

namespace sala::sym
{

struct StateStore
{
    void add( ExecState *state )
    {
        _states.insert( state );
    }

    void remove( ExecState *state )
    {
        _states.erase( state );
    }

    bool empty() const
    {
        return _states.empty();
    }

private:

    std::set< ExecState *, ExecStateIDCmp > _states;
};

} // namespace sala::sym
