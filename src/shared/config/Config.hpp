
#pragma once

namespace flecs
{
    struct world;
}

namespace wb
{
    struct ConfigSystem
    {
        ConfigSystem( flecs::world & world );
    };
} // namespace wb
