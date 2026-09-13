#pragma once

#include "salasym/config.hpp"
#include "salasym/state.hpp"

#include <memory>
#include <span>

namespace sala::sym
{

struct ISearcher
{
    virtual ~ISearcher() = default;

    virtual ExecState &select() = 0;

    virtual bool empty() const = 0;

    virtual void update( ExecState *curr,
            const std::span< ExecState * > &added,
            const std::span< ExecState * > &removed ) = 0;
};

ptr< ISearcher > make_searcher( const SearchConfig &config );

} // namespace sala::sym
