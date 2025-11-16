
#pragma once

#include <fmt/base.h>
#include <spdlog/common.h>

#include <cstdint>
#include <filesystem>
#include <memory>

namespace spdlog
{
    namespace sinks
    {
        class sink;
    }

    class logger;
} // namespace spdlog

namespace shm
{
    // TODO: Probably could be nice to support multiple sinks, right now it always creates a rotating file sink
    // some variant-like?
    struct LoggingInitData
    {
        std::string m_log_file_name{ "DEFAULT_CONFIG_LOG_RENAME_ME_IN_LOGGING_INIT_DATA.log" };
        std::filesystem::path m_log_file_path{ "./logs/" };
        uint32_t m_max_file_size_bytes{ 1024 * 1024 * 5 }; // 5 MB
        uint32_t m_max_files{ 5 };
    };

    struct Logger;
    struct Sink
    {
        Sink() = delete;

        virtual ~Sink();

        Sink( const Sink & )             = delete;
        Sink( Sink && )                  = delete;
        Sink & operator=( const Sink & ) = delete;
        Sink & operator=( Sink && )      = delete;

        static std::unique_ptr< shm::Sink > CreateLogSink( LoggingInitData && lid );

        std::unique_ptr< Logger > AttachLogger( std::string_view logger_name,
                                                spdlog::level::level_enum log_level,
                                                spdlog::level::level_enum flush_level,
                                                std::string_view log_pattern = "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l@%t]: %v" );

    private:
        explicit Sink( std::shared_ptr< spdlog::sinks::sink > && spdlog_sink );
        std::shared_ptr< spdlog::sinks::sink > m_spdlog_sink;
    };

    struct Logger
    {
        Logger() = delete;
        virtual ~Logger();

        Logger( const Logger & )             = delete;
        Logger( Logger && )                  = delete;
        Logger & operator=( const Logger & ) = delete;
        Logger & operator=( Logger && )      = delete;

        template< typename... Args >
        void Log( spdlog::level::level_enum log_lvl, fmt::format_string< Args... > fmt, Args &&... args )
        {
            SinkMessage( log_lvl, fmt, fmt::make_format_args( args... ) );
        }

    protected:
        void SinkMessage( spdlog::level::level_enum log_lvl,
                          fmt::string_view fmt,
                          fmt::format_args && args );

    private:
        // Grant Sink access so AttachLogger can construct Logger
        friend struct Sink;

        explicit Logger( std::shared_ptr< spdlog::logger > && spdlog_logger );
        std::shared_ptr< spdlog::logger > m_spdlog_logger;
    };

} // namespace shm