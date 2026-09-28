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

    virtual ExecState &take() = 0;

    virtual bool empty() const = 0;

    virtual void publish( std::span< ExecState * const > ordered ) = 0;
};

ptr< ISearcher > make_searcher( const SearchConfig &config );

} // namespace sala::sym
