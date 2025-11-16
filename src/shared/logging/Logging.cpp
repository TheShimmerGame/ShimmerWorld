
#include "Logging.hpp"

#include <spdlog/logger.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

shm::Logger::~Logger() = default;

shm::Logger::Logger( std::shared_ptr< spdlog::logger > && spdlog_logger )
    : m_spdlog_logger( std::move( spdlog_logger ) )
{
}

void shm::Logger::SinkMessage( spdlog::level::level_enum log_lvl, fmt::string_view fmt, fmt::format_args && args )
{
    m_spdlog_logger->log( log_lvl, fmt::vformat( fmt, args ) );
}

shm::Sink::~Sink() = default;

shm::Sink::Sink( std::shared_ptr< spdlog::sinks::sink > && spdlog_sink )
    : m_spdlog_sink( std::move( spdlog_sink ) )
{
}

std::unique_ptr< shm::Sink > shm::Sink::CreateLogSink( LoggingInitData && lid )
{
    auto logPath       = lid.m_log_file_path / lid.m_log_file_name;
    auto rotating_sink = std::make_shared< spdlog::sinks::rotating_file_sink_mt >( logPath.string(),
                                                                                   lid.m_max_file_size_bytes,
                                                                                   lid.m_max_files,
                                                                                   true );

    rotating_sink->set_pattern( "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l@%t]: %v" );

    // By default we're getting stdcerr sink attached to the logger as well
    // and MSVC sink on Windows (till i have an ui with logging window)
    // TODO: get rid of this
    std::vector< spdlog::sink_ptr > common_sinks;
#ifdef _WIN32
    auto msvc_sink = std::make_shared< spdlog::sinks::msvc_sink_mt >();
    common_sinks.push_back( msvc_sink );
#endif
    common_sinks.push_back( rotating_sink );
    auto stdcerr_console_sink = std::make_shared< spdlog::sinks::stderr_color_sink_mt >();
    common_sinks.push_back( stdcerr_console_sink );

    rotating_sink->set_level( spdlog::level::trace ); // TODO argument in logging init data
    return std::unique_ptr< shm::Sink >( new shm::Sink( std::move( rotating_sink ) ) );
}

std::unique_ptr< shm::Logger > shm::Sink::AttachLogger( std::string_view logger_name, spdlog::level::level_enum log_level, spdlog::level::level_enum flush_level, std::string_view log_pattern /*= "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l@%t]: %v" */ )
{
    auto logger = std::make_shared< spdlog::logger >( std::string( logger_name ), m_spdlog_sink );
    logger->set_level( log_level );
    logger->flush_on( flush_level );
    logger->set_pattern( std::string{ log_pattern } );
    return std::unique_ptr< shm::Logger >( new shm::Logger( std::move( logger ) ) );
}
