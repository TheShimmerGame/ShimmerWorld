#include "Logging.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace
{
    thread_local std::vector< std::string > S_THREAD_SCOPES;

    // Custom flag formatter for {settings.ScopePatternFlag}: emits "[A::B]: " when scopes exist.
    class scope_flag_formatter final : public spdlog::custom_flag_formatter
    {
    public:
        void format( const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t & dest ) override
        {
            if ( !S_THREAD_SCOPES.empty() )
                fmt::format_to( std::back_inserter( dest ), "[{}]: ", fmt::join( S_THREAD_SCOPES, "::" ) );
        }

        std::unique_ptr< spdlog::custom_flag_formatter > clone() const override
        {
            return std::unique_ptr< spdlog::custom_flag_formatter >( new scope_flag_formatter() );
        }
    };
} // namespace

namespace shm
{
    LogScope::LogScope( std::string_view scope_name )
    {
        S_THREAD_SCOPES.emplace_back( scope_name );
    }

    LogScope::~LogScope()
    {
        if ( !S_THREAD_SCOPES.empty() )
            S_THREAD_SCOPES.pop_back();
    }

    Logger::Logger( const LoggerSettings & settings )
    {
        if ( settings.m_create_directories )
            std::filesystem::create_directories( settings.m_log_file_path, m_directory_ec );

        if ( !m_directory_ec )
        {
            const auto log_path = std::filesystem::path( settings.m_log_file_path ) / settings.m_log_file_name;
            std::vector< spdlog::sink_ptr > sinks;

            auto rotating_sink = std::make_shared< spdlog::sinks::rotating_file_sink_mt >(
                log_path.string(),
                settings.m_max_file_size_bytes,
                settings.m_max_files,
                settings.m_rotate_on_open );
            sinks.push_back( rotating_sink );

#ifdef _WIN32
            if ( settings.m_enable_msvc )
                sinks.push_back( std::make_shared< spdlog::sinks::msvc_sink_mt >() );
#endif

            if ( settings.m_enable_stderr )
            {
                sinks.push_back( std::make_shared< spdlog::sinks::stderr_color_sink_mt >() );
            }

            // Create logger and configure
            auto logger = std::make_shared< spdlog::logger >( settings.m_logger_name, sinks.begin(), sinks.end() );
            logger->set_level( settings.m_level );
            logger->flush_on( settings.m_flush_level );

            {
                auto pf = std::make_unique< spdlog::pattern_formatter >();
                pf->add_flag< scope_flag_formatter >( '*' );
                pf->set_pattern( settings.m_log_pattern );
                logger->set_formatter( std::move( pf ) );
            }

            for ( auto & s : sinks )
                s->set_level( settings.m_level );

            m_logger = std::move( logger );

            spdlog::set_default_logger( m_logger );
        }
    }

    Logger::~Logger()
    {
        spdlog::drop_all();
        spdlog::shutdown();
    }

} // namespace shm
