
#pragma once

#include <expected>

namespace shm
{
    template< typename T >
    using Result = std::expected< T, std::error_code >;
} // namespace shm
