#pragma once

#include "salasym/target.hpp"
#include "salasym/types.hpp"

#include <cstdint>
#include <span>
#include <string>

namespace llvm
{
class APInt;
}

namespace sala::sym
{

enum class ExprOp : u8
{
    Const,

    Address,

    Add,
    Sub,
    Mul,

    SLt,
    SGt,
    Eq,
    LNot,

    ZExt,
    Trunc,

    Symbol,
};

struct Expr
{
    static Expr uconst( u64 value, u32 width );
    static Expr sconst( i64 value, u32 width );
    static Expr constant( std::span< const u8 > bytes, u32 width,
        ByteOrder byte_order );

    static Expr symbol( std::string name, u32 width );

    static Expr add( const Expr &lhs, const Expr &rhs );
    static Expr sub( const Expr &lhs, const Expr &rhs );
    static Expr mul( const Expr &lhs, const Expr &rhs );

    static Expr slt( const Expr &lhs, const Expr &rhs );
    static Expr sgt( const Expr &lhs, const Expr &rhs );
    static Expr eq( const Expr &lhs, const Expr &rhs );
    static Expr logical_not( const Expr &src );

    static Expr zext( const Expr &src, u32 width );
    static Expr trunc( const Expr &source, u32 width );

    static Expr address( ObjId id, u64 offset );
    Pointer as_pointer() const;

    u32 width() const;
    std::string to_string() const;

private:

    struct Node;
    ref< Node > _node;

    Expr( ExprOp op, llvm::APInt value );
    Expr( ExprOp op, u64 raw, u32 width );
    Expr( ExprOp op, std::string name, u32 width );
    Expr( ExprOp op, u32 width, const Expr &lhs, const Expr &rhs );
    Expr( ExprOp op, u32 width, const Expr &operand );

    friend const Node &node( const Expr &expr );
};

} // namespace sala::sym
