#include "salasym/expr.hpp"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/SmallString.h>

#include "utility/invariants.hpp"

#include <cstddef>
#include <limits>
#include <utility>

namespace sala::sym
{

struct Expr::Node
{
    ExprOp op;

    u32 width;
    u64 raw;
    llvm::APInt value;
    std::string name;

    ref< Node > lhs;
    ref< Node > rhs;

    Node( ExprOp op, llvm::APInt value )
        : op( op )
        , width( value.getBitWidth() )
        , raw( 0 )
        , value( std::move( value ) )
    {}

    Node( ExprOp op, u64 raw, u32 width )
        : op( op )
        , width( width )
        , raw( raw )
        , value( 1, 0 )
    {}

    Node( ExprOp op, std::string name, u32 width )
        : op( op )
        , width( width )
        , raw( 0 )
        , value( 1, 0 )
        , name( std::move( name ) )
    {}

    Node( ExprOp op, u32 width, ref< Node > lhs, ref< Node > rhs = nullptr )
        : op( op )
        , width( width )
        , raw( 0 )
        , value( 1, 0 )
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
            case ExprOp::Const:
            {
                llvm::SmallString< 32 > text;
                value.toStringUnsigned( text );
                return std::string( text.data(), text.size() );
            }
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
            case ExprOp::Trunc:   return "trunc(" + lhs->to_string() + ")";
        }

        UNREACHABLE();
    }
};

const Expr::Node &node( const Expr &expr )
{
    return *expr._node;
}

Expr::Expr( ExprOp op, llvm::APInt value )
    : _node( make_ref< Node >( op, std::move( value ) ) )
{}

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

Expr Expr::uconst( u64 value, u32 width )
{
    INVARIANT( width > 0 );
    INVARIANT( width >= 64 || llvm::isUIntN( width, value ) );
    return Expr( ExprOp::Const, llvm::APInt( width, value ) );
}

Expr Expr::sconst( i64 value, u32 width )
{
    const u64 raw = static_cast< u64 >( value );

    INVARIANT( width > 0 );
    INVARIANT( width >= 64 || llvm::isIntN( width, raw ) );
    return Expr( ExprOp::Const, llvm::APInt( width, raw, true ) );
}

Expr Expr::constant( std::span< const u8 > bytes, u32 width,
    ByteOrder byte_order )
{
    INVARIANT( width > 0 );
    INVARIANT( byte_order == ByteOrder::LittleEndian );

    constexpr u64 u8_bits = std::numeric_limits< u8 >::digits;
    const u64 byte_count = ( width + u8_bits - 1 ) / u8_bits;
    INVARIANT( bytes.size() == byte_count );

    const u32 last_bits = width % u8_bits;
    if ( last_bits != 0 )
    {
        const u8 unused = ~( ( u32( 1 ) << last_bits ) - 1 );
        INVARIANT( ( bytes.back() & unused ) == 0 );
    }

    llvm::APInt value( width, 0 );

    for ( u32 i = 0; i < byte_count; ++i )
    {
        const bool last = i + 1 == byte_count;
        const u32 bit_pos = i * u8_bits;
        const u32 bits = last ? width - bit_pos : u8_bits;

        value.insertBits( bytes[ i ], bit_pos, bits );
    }

    return Expr( ExprOp::Const, std::move( value ) );
}

Expr Expr::trunc( const Expr &source, u32 width )
{
    const auto &source_node = node( source );

    INVARIANT( width > 0 );
    INVARIANT( width < source.width() );

    if ( source_node.op == ExprOp::Const )
        return Expr( ExprOp::Const, source_node.value.trunc( width ) );

    return Expr( ExprOp::Trunc, width, source );
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
