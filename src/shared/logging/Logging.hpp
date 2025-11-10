
#pragma once

#include "LevelsCategories.hpp"

#include <fmt/format.h>

namespace shm
{
    struct LoggingConfig;

    void InitLogging( const shm::LoggingConfig & config, std::string_view log_file_name );
    void ShutdownLogging();

    void SinkMessage( shm::LogCategory category,
                      shm::LogLevel level,
                      fmt::string_view fmt,
                      fmt::format_args args );

    template< typename... Args >
    void Log( shm::LogLevel level, fmt::format_string< Args... > fmt, Args &&... args )
    {
        SinkMessage( shm::LogCategory::General, level, fmt, fmt::make_format_args( args... ) );
    }

    template< typename... Args >
    void Log( shm::LogCategory category,
              shm::LogLevel level,
              fmt::format_string< Args... > fmt,
              Args &&... args )
    {
        SinkMessage( category, level, fmt, fmt::make_format_args( args... ) );
    }

} // namespace shm
