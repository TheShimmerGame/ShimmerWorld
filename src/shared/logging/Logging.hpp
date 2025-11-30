#pragma once

#include <spdlog/common.h>
#include "results/Result.hpp"

#include <memory>
#include <string>
#include <vector>

namespace spdlog
{
    class logger;
    namespace sinks
    {
        class sink;
    }
} // namespace spdlog

namespace shm
{
    /// @brief RAII scope logger. On construction, pushes scope name to thread-local stack.
    /// When destroyed, pops scope name from stack.
    /// It is visible in log messages via the %Z pattern flag.
    struct LogScope
    {
        LogScope() = delete;
        explicit LogScope( std::string_view scope_name );
        ~LogScope();

        LogScope( const LogScope & )             = delete;
        LogScope & operator=( const LogScope & ) = delete;
        LogScope( LogScope && )                  = delete;
        LogScope & operator=( LogScope && )      = delete;
    };

    struct LoggerSettings
    {
        std::string m_log_file_name{ "DEFAULT_CONFIG_LOG_RENAME_ME_IN_LOGGING_INIT_DATA.log" };
        std::string m_log_file_path{ "./logs/" };
        uint32_t m_max_file_size_bytes{ 1024u * 1024u * 5u }; // 5 MB
        uint32_t m_max_files{ 5u };
        bool m_rotate_on_open{ true };
        bool m_create_directories{ true };

        std::string m_logger_name{ "shimmer" };
        spdlog::level::level_enum m_level{ spdlog::level::info };
        spdlog::level::level_enum m_flush_level{ spdlog::level::info };
        /// @brief Log pattern, using spdlog pattern syntax. Controls how log messages are formatted.
        std::string m_log_pattern{ "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l@%t] %*%v" };

        bool m_enable_stderr{ true };
        // TODO get rid of this preprocessor altogether once we have UI layer
#ifdef _WIN32
        bool m_enable_msvc{ true };
#else
        bool m_enable_msvc{ false };
#endif
    };

    struct Logger
    {
        explicit Logger( const LoggerSettings & settings );
        ~Logger();

        Logger()                             = delete;
        Logger( const Logger & )             = delete;
        Logger & operator=( const Logger & ) = delete;
        Logger( Logger && )                  = delete;
        Logger & operator=( Logger && )      = delete;

        shm::Result< void > GetDirectoryCreationError() const
        {
            if ( m_directory_ec )
                return std::unexpected( m_directory_ec );
            return {};
        }

    private:
        std::error_code m_directory_ec;
        std::shared_ptr< spdlog::logger > m_logger;
    };

} // namespace shm
