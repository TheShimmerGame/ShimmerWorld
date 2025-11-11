
#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

namespace shm
{
    enum LogLevel : uint8_t;
    enum LogCategory : uint8_t;

    struct ConfigPathComponent
    {
        std::filesystem::path m_config_path;
    };

    struct NetworkConfigComponent
    {
        // we need the component to actually have size, flecs requirements, TODO: fill in with actual data
        uint32_t temporary = 0;
    };

    struct DatabaseConfigComponent
    {
        // we need the component to actually have size, flecs requirements, TODO: fill in with actual data
        uint32_t temporary = 0;
    };

    struct CategoryLogLevel
    {
        shm::LogCategory m_category;
        shm::LogLevel m_level;
    };

    struct LoggingConfigComponent
    {
        std::vector< CategoryLogLevel > m_category_levels;
    };

} // namespace shm