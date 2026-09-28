#pragma once

#include "salasym/search.hpp"

#include "utility/invariants.hpp"

#include <vector>

namespace sala::sym
{

struct DFS_Searcher : public ISearcher
{
    ExecState &take() override
    {
        INVARIANT( !empty() );

        ExecState *state = _states.back();
        _states.pop_back();
        return *state;
    }

    bool empty() const override
    {
        return _states.empty();
    }

    void publish( std::span< ExecState * const > ordered ) override
    {
        _states.insert( _states.end(), ordered.rbegin(), ordered.rend() );
    }

private:

    std::vector< ExecState * > _states;
};

} // namespace sala::sym