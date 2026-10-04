#pragma once

#include "salasym/expr.hpp"
#include "salasym/types.hpp"

#include "utility/invariants.hpp"

#include <unordered_map>
#include <vector>

namespace sala::sym
{

struct MemObj
{
    std::vector< Expr > cells;
};

struct Memory
{
    const Expr &load( Pointer p ) const
    {
        return at( p );
    }

    void store( Pointer p, Expr value )
    {
        at( p ) = std::move( value );
    }

    void create( ObjId id, u32 width )
    {
        INVARIANT( !_objects.contains( id ) );
        _objects[ id ] = MemObj{ { Expr::uconst( 0, width ) } };
    }

private:

    std::unordered_map< ObjId, MemObj > _objects;

    template< typename Self >
    static auto &at( Self &&self, Pointer p )
    {
        auto it = self._objects.find( p.obj );
        INVARIANT( it != self._objects.end() );

        auto &obj = it->second;
        INVARIANT( p.offset < obj.cells.size() );

        return obj.cells[ p.offset ];
    }

          Expr &at( Pointer p )       { return at( *this, p ); }
    const Expr &at( Pointer p ) const { return at( *this, p ); }
};

} // namespace sala::sym
