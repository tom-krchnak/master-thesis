#pragma once

#include "salasym/expr.hpp"
#include "salasym/types.hpp"

#include "utility/invariants.hpp"

#include <array>
#include <string_view>
#include <variant>
#include <vector>

namespace sala::sym
{

enum class StopKind
{
    Return,
    Halt,
    Unsupported,
};

enum class UnsupportedReason
{
    None,
    Opcode,
    Modifier,
    Operand,
    ExternalModel,
};

struct Stop
{
    StopKind kind;
    UnsupportedReason reason = UnsupportedReason::None;
};

struct Advance {};
struct Return {};

struct Jump
{
    u32 block;
};

struct Call
{
    u32 callee;
    std::vector< Expr > args;
};

struct GuardedContinuation
{
    Expr cond;
    u32 block;
};

struct Split
{
    std::array< GuardedContinuation, 2 > alts;
};

using StepOutcome = std::variant< Advance, Return, Jump, Call, Split, Stop >;

inline std::string_view to_str( StopKind kind )
{
    switch ( kind )
    {
        case StopKind::Return:      return "return";
        case StopKind::Halt:        return "halt";
        case StopKind::Unsupported: return "unsupported";
    }

    UNREACHABLE();
}

} // namespace sala::sym
