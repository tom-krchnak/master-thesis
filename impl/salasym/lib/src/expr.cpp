#include "salasym/expr.hpp"

#include "utility/invariants.hpp"

namespace sala::sym
{

struct Expr::Node
{
    ExprOp op;

    u32 width;
    u64 raw;
    std::string name;

    ref< Node > lhs;
    ref< Node > rhs;

    Node( ExprOp op, u64 raw, u32 width )
        : op( op )
        , width( width )
        , raw( raw )
    {}

    Node( ExprOp op, std::string name, u32 width )
        : op( op )
        , width( width )
        , name( std::move( name ) )
    {}

    Node( ExprOp op, u32 width, ref< Node > lhs, ref< Node > rhs = nullptr )
        : op( op )
        , width( width )
        , lhs( std::move( lhs ) )
        , rhs( std::move( rhs ) )
    {}

    std::string to_string() const
    {
        const auto binary = [ this ]( const char *op )
        {
            return "(" + lhs->to_string() + " " + op + " " + rhs->to_string() + ")";
        };

        switch ( op )
        {
            case ExprOp::Const:   return std::to_string( raw );
            case ExprOp::Symbol:  return name;
            case ExprOp::Address: return "&obj" + std::to_string( raw >> 32 )
                                          + "+" + std::to_string( raw & 0xffff'ffff );
            case ExprOp::Add:     return binary( "+" );
            case ExprOp::Sub:     return binary( "-" );
            case ExprOp::Mul:     return binary( "*" );
            case ExprOp::SLt:     return binary( "<" );
            case ExprOp::SGt:     return binary( ">" );
            case ExprOp::Eq:      return binary( "==" );
            case ExprOp::LNot:    return "!(" + lhs->to_string() + ")";
            case ExprOp::ZExt:    return "zext(" + lhs->to_string() + ")";
        }

        UNREACHABLE();
    }
};

const Expr::Node &node( const Expr &expr )
{
    return *expr._node;
}

Expr::Expr( ExprOp op, u64 raw, u32 width )
    : _node( make_ref< Node >( op, raw, width ) )
{}

Expr::Expr( ExprOp op, std::string name, u32 width )
    : _node( make_ref< Node >( op, std::move( name ), width ) )
{}

Expr::Expr( ExprOp op, u32 width, const Expr &lhs, const Expr &rhs )
    : _node( make_ref< Node >( op, width, lhs._node, rhs._node ) )
{}

Expr::Expr( ExprOp op, u32 width, const Expr &operand )
    : _node( make_ref< Node >( op, width, operand._node ) )
{}

// accessors

u32 Expr::width() const
{
    return node( *this ).width;
}

// operations

Expr Expr::constant( u64 raw, u32 width )
{
    return Expr( ExprOp::Const, raw, width );
}

Expr Expr::symbol( std::string name, u32 width )
{
    return Expr( ExprOp::Symbol, std::move( name ), width );
}

Expr Expr::add( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    // TODO: constant folding
    // TODO: optimizations: i.e. unit (x + 0 = x)
    return Expr( ExprOp::Add, lhs.width(), lhs, rhs );
}

Expr Expr::sub( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    return Expr( ExprOp::Sub, lhs.width(), lhs, rhs );
}

Expr Expr::mul( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    return Expr( ExprOp::Mul, lhs.width(), lhs, rhs );
}

Expr Expr::slt( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    return Expr( ExprOp::SLt, 8, lhs, rhs );
}

Expr Expr::sgt( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    return Expr( ExprOp::SGt, 8, lhs, rhs );
}

Expr Expr::eq( const Expr &lhs, const Expr &rhs )
{
    INVARIANT( lhs.width() == rhs.width() );
    return Expr( ExprOp::Eq, 8, lhs, rhs );
}

Expr Expr::logical_not( const Expr &src )
{
    return Expr( ExprOp::LNot, src.width(), src );
}

Expr Expr::zext( const Expr &src, u32 width )
{
    INVARIANT( width >= src.width() );
    return Expr( ExprOp::ZExt, width, src );
}


Expr Expr::address( ObjId id, u64 offset )
{
    u64 raw = ( u64( id ) << 32 | offset );
    return Expr( ExprOp::Address, raw, 64 );
}

Pointer Expr::as_pointer() const
{
    INVARIANT( node( *this ).op == ExprOp::Address );
    u64 raw = node( *this ).raw;
    return { ObjId( raw >> 32 ), raw & 0xffff'ffff };
}

std::string Expr::to_string() const
{
    return node( *this ).to_string();
}

} // namespace sala::sym
