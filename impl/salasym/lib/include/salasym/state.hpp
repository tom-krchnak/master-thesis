#pragma once

#include "salasym/memory.hpp"
#include "salasym/path_cond.hpp"
#include "salasym/types.hpp"

#include <cstddef>
#include <optional>

namespace sala::sym
{

struct StateStore;

struct StateId
{
    explicit StateId( u32 id )
        : _id( id )
    {}

    bool  operator==( const StateId &other ) const = default;
    auto operator<=>( const StateId &other ) const = default;

private:

    u32 _id;
};

struct ProgramLocation
{
    u32 funct;
    u32 block;
    u32 instr;
};

struct Frame
{
    ProgramLocation loc;
    std::optional< ProgramLocation > call_site;

    Pointer param( u32 idx ) const { return { _params[ idx ], 0 }; }
    Pointer local( u32 idx ) const { return { _locals[ idx ], 0 }; }

    void push_param( ObjId id ) { _params.push_back( id ); }
    void push_local( ObjId id ) { _locals.push_back( id ); }

private:

    std::vector< ObjId > _params;
    std::vector< ObjId > _locals;
};


struct ExecState
{
    explicit ExecState( StateId id )
        : _id( id )
    {}

    StateId id() const { return _id; }

    std::vector< Frame > frames;
    Memory memory;
    PathCondition path;

private:

    StateId _id;
    std::size_t _store_slot = 0;
    friend struct StateStore;
};

} // namespace sala::sym
