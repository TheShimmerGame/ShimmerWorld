
#include "Logging.hpp"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <iostream>
#include <print>

static std::unordered_map< shm::LogCategory, std::shared_ptr< spdlog::logger > > G_Loggers;
void shm::InitLogging( const shm::LoggingConfig & config, std::string_view log_file_name )
{
    std::filesystem::path logPath{ std::string{ log_file_name } };
    if ( logPath.has_parent_path() )
        std::filesystem::create_directories( logPath.parent_path() );

    auto rotating_sink = std::make_shared< spdlog::sinks::rotating_file_sink_mt >( std::string{ log_file_name }, 1024 * 1024, 5, true );
    // [LoggerName] [Timestamp] [LogLevel] Message
    rotating_sink->set_pattern( "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l@%t]: %v" );

    // output everything to the console atm
    // TODO: Get rid of this once we have stable tui
    auto stdcerr_console_sink = std::make_shared< spdlog::sinks::stderr_color_sink_mt >();
    stdcerr_console_sink->set_pattern( "[%n] [%H:%M:%S] [%^%l%$] %v" );

    std::vector< spdlog::sink_ptr > common_sinks;
    // TODO get rid of preprocessor and make it configurable via config file
#ifdef _WIN32
    auto msvc_sink = std::make_shared< spdlog::sinks::msvc_sink_mt >();
    msvc_sink->set_pattern( "[%n] [%Y-%m-%d %H:%M:%S.%e] [%l]: %v" );
    common_sinks.push_back( msvc_sink );
#endif

    common_sinks.push_back( rotating_sink );
    common_sinks.push_back( stdcerr_console_sink );

    // for now set log level to debug and create per-category loggers that use all sinks
    // TODO: Get debug levels from config
    magic_enum::enum_for_each< shm::LogCategory >( [ & ]( auto category )
                                                   {
                                                       auto enum_name = magic_enum::enum_name( static_cast< shm::LogCategory >( category ) );
                                                       auto logger    = std::make_shared< spdlog::logger >( std::string{ enum_name },
                                                                                                            common_sinks.begin(),
                                                                                                            common_sinks.end() );
                                                       logger->set_level( spdlog::level::trace );
                                                       spdlog::register_logger( logger );
                                                       G_Loggers[ static_cast< shm::LogCategory >( category ) ] = std::move( logger );
                                                   } );

    // set default logger, in case we ever need to call spdlog::* functions
    // but our logs go thorugh SinkMessage
    // spdlog::set_default_logger( std::make_shared< spdlog::logger >( "default_logger", common_sinks.begin(), common_sinks.end() ) );

    spdlog::flush_on( spdlog::level::warn );
}

void shm::ShutdownLogging()
{
    // flush the spdlog just in case
    for ( auto & [ category, logger ] : G_Loggers )
    {
        logger->flush();
    }

    G_Loggers.clear();
}

spdlog::level::level_enum SpdlogLevelFromShmLevel( shm::LogLevel lvl )
{
    return static_cast< spdlog::level::level_enum >( std::to_underlying( lvl ) );
}

void shm::SinkMessage( shm::LogCategory category, shm::LogLevel level, fmt::string_view fmt, fmt::format_args args )
{
    auto & logger = G_Loggers[ category ];
    if ( logger )
        logger->log( SpdlogLevelFromShmLevel( level ), fmt::vformat( fmt, args ) );
}
