#pragma once

#include "salasym/search.hpp"

#include "utility/invariants.hpp"

#include <algorithm>
#include <vector>

namespace sala::sym
{

struct DFS_Searcher : public ISearcher
{
    ExecState &select() override
    {
        INVARIANT( !empty() );
        return *_states.back();
    }

    bool empty() const override
    {
        return _states.empty();
    }

    void update( ExecState *curr,
            const std::span< ExecState * > &added,
            const std::span< ExecState * > &removed ) override
    {
        for ( ExecState *state : removed )
        {
            auto it = std::find( _states.begin(), _states.end(), state );
            INVARIANT( it != _states.end() );
            _states.erase( it );
        }

        _states.insert( _states.end(), added.begin(), added.end() );
    }

private:

    std::vector< ExecState * > _states;
};

} // namespace sala::sym