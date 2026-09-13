#pragma once

#include <cstdint>
#include <memory>

namespace sala::sym
{

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;


template< typename T >
using ptr = std::unique_ptr< T >;

template< typename T >
using ref = std::shared_ptr< T >;

template< typename T, typename... Args >
auto make_ref( Args&&... args )
{
    return std::make_shared< T >( std::forward< Args >( args )... );
}


using ObjId = u32;

struct Pointer
{
    ObjId obj;
    u64 offset;
};

} // namespace sala::sym
