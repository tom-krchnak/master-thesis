#pragma once

#include "salasym/types.hpp"

#include <string>

namespace sala::sym
{

enum class ExprOp : u8
{
    Const,

    Address,

    Add,
    Sub,

    SGt,

    ZExt,

    Symbol,
};

struct Expr
{
    static Expr constant( u64 raw, u32 width );
    static Expr symbol( std::string name, u32 width );

    static Expr add( const Expr &lhs, const Expr &rhs );

    static Expr sgt( const Expr &lhs, const Expr &rhs );
    static Expr zext( const Expr &src, u32 width );

    static Expr address( ObjId id, u64 offset );
    Pointer as_pointer() const;

    u32 width() const;
    std::string to_string() const;

private:

    struct Node;
    ref< Node > _node;

    Expr( ExprOp op, u64 raw, u32 width );
    Expr( ExprOp op, std::string name, u32 width );
    Expr( ExprOp op, u32 width, const Expr &lhs, const Expr &rhs );
    Expr( ExprOp op, u32 width, const Expr &operand );

    friend const Node &node( const Expr &expr );
};

} // namespace sala::sym
