#pragma once

#include "salasym/expr.hpp"

#include <vector>

namespace sala::sym
{

struct PathCondition
{
    void add( Expr constraint )
    {
        _constraints.push_back( std::move( constraint ) );
    }

private:

    std::vector< Expr > _constraints;
};

} // namespace sala::sym
