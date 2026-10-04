#pragma once

#include "salasym/types.hpp"

#include "utility/invariants.hpp"

#include <string_view>

namespace sala::sym
{

enum class ByteOrder : u8
{
    LittleEndian,
    BigEndian,
};

struct TargetProfile
{
    TargetProfile( const sala::Program &program )
        : _byte_order( ByteOrder::LittleEndian )
        , _ptr_width( 64 )
        , _int_width( 32 )
    {
        INVARIANT( program.num_cpu_bits() == 64 );
        INVARIANT( is_x86_64_linux( program.system() ) );
    }

    ByteOrder byte_order() const noexcept { return _byte_order; }
    u32 ptr_width() const noexcept { return _ptr_width; }
    u32 int_width() const noexcept { return _int_width; }

private:

    static bool is_x86_64_linux( std::string_view system )
    {
        return system.starts_with( "x86_64-" )
            && system.find( "linux" ) != std::string_view::npos;
    }

    ByteOrder _byte_order;
    u32 _ptr_width;
    u32 _int_width;
};

} // namespace sala::sym
