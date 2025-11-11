
#pragma once

#include <cstdint>

namespace shm
{
    /* @brief Log levels directly translate to spdlog log levels
    namespace level
    {
        enum level_enum : int {
        trace = SPDLOG_LEVEL_TRACE,
        debug = SPDLOG_LEVEL_DEBUG,
        info = SPDLOG_LEVEL_INFO,
        warn = SPDLOG_LEVEL_WARN,
        err = SPDLOG_LEVEL_ERROR,
        critical = SPDLOG_LEVEL_CRITICAL,
        off = SPDLOG_LEVEL_OFF,
        n_levels
    };
    */
    /// @brief Represents the severity of the log message. Unscoped enum for easier use in shm::Log function
    /// e.g. shm::Log( shm::Info, "Message {} ", var ); instead of shm::Log( shm::LogLevel::Info, "Message {} ", var );
    enum LogLevel : uint8_t
    {
        Trace    = 0,
        Debug    = 1,
        Info     = 2,
        Warn     = 3,
        Error    = 4,
        Critical = 5,
        Off      = 6
    };

    /// @brief Each category represents a different logger and each logger can have it's own log level
    /// Log levels are controlled via configuration files ( see config/ConfigComponents.hpp )
    /// When creating new logger, make sure to create an actual spdlog object in shm::log::InitLogging (Logging.cpp)
    /// Unscoped enum for easier use in shm::Log function, e.g. shm::Log( shm::Info, "Message {} ", var ); instead of
    /// shm::Log( shm::LogLevel::Info, "Message {} ", var );
    enum LogCategory : uint8_t
    {
        General = 0,
        Network = 1
    };
} // namespace shm
