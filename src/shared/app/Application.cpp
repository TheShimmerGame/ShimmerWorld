#include "Application.hpp"

#include <args.hxx>
#include <flecs.h>

#include <filesystem>
#include <iostream>
#include <print>

#include "logging/Logging.hpp"
#include <spdlog/spdlog.h>

#include <config/Config.hpp>

struct TestConfig
{
    static constexpr uint32_t ConfigVersion = 1;

    int32_t some_integer = 42;
};

namespace
{
    shm::Result< TestConfig > TestConfigMigrator( std::string & json_data, uint32_t from_version,
                                                  uint32_t to_version )
    {
        return TestConfig{};
    }
} // namespace

wb::Application::Application()
    : m_broker_world( std::make_unique< flecs::world >() )
    , m_logger( nullptr )
{
}

wb::Application::~Application() = default;

constexpr uint32_t TARGET_FPS = 120;

int wb::Application::Run( int argc, char ** argv )
{
    std::string config_directory = "./configs";
    args::ArgumentParser parser( "Shimmer World Broker Application",
                                 "A broker application for managing connections from players and service servers." );
    try
    {
        args::HelpFlag help( parser, "help", "Display this help menu", { 'h', "help" } );
        args::ValueFlag< std::string > config_dir( parser, "config-dir", "Directory to load configuration files from",
                                                   { 'c', "config-dir" }, args::Options::Single );
        parser.ParseCLI( argc, argv );

        if ( config_dir )
            config_directory = config_dir.Get();
    }
    catch ( const args::Completion & )
    {
        return 0;
    }
    catch ( const args::Help & h )
    {
        std::print( std::cerr, "{}\n{}", h.what(), parser.Help() );
        return 0;
    }
    catch ( args::ParseError e )
    {
        std::print( std::cerr, "{}\n{}", e.what(), parser.Help() );
        return 1;
    }
    catch ( args::ValidationError e )
    {
        std::print( std::cerr, "{}\n{}", e.what(), parser.Help() );
        return 1;
    }

    if ( !InitializeLoggingSystem() )
    {
        std::println( "Failed to initialize logging system, shutting down." );
        return 1;
    }

    spdlog::info( "Application starting {}", 1 );

    {
        shm::LogScope startup{ "Startup" };
        spdlog::debug( "Loading configuration" );
        {
            shm::LogScope cfg{ "Config" };
            spdlog::info( "Using directory: {}", config_directory );
        }
    }

    shm::Config cfg{ config_directory };
    if ( !cfg.IsDirectoryCreated() )
    {
        spdlog::error( "Failed to create configuration directory: {}", config_directory );
        return 1;
    }

    auto test_result = cfg.RegisterConfig( TestConfig{}, "TestConfig", "json", &TestConfigMigrator );
    {
        auto test_cfg          = cfg.GetConfig< TestConfig >( "TestConfig" );
        test_cfg->some_integer = 12345;
    }
    auto all_results = cfg.SaveDirtyConfigs();

    for ( const auto & res : all_results )
    {
        if ( !res.has_value() )
        {
            spdlog::error( "Failed to save config: {}", res.error().message() );
        }
    }

    return 0;
}

bool wb::Application::InitializeLoggingSystem()
{

    shm::LoggerSettings logger_settings{
        .m_log_file_name       = "worldbroker.log",
        .m_log_file_path       = "./logs/",
        .m_max_file_size_bytes = 1024 * 1024 * 5,
        .m_max_files           = 5,
        .m_rotate_on_open      = true,
        .m_create_directories  = true,
        .m_logger_name         = "worldbroker",
        .m_level               = spdlog::level::debug,
        .m_flush_level         = spdlog::level::info,
        .m_log_pattern         = R"([%Y-%m-%d %H:%M:%S.%e] [%l@thread:%t] %*%v)",
        .m_enable_stderr       = true,
#ifdef _WIN32
        .m_enable_msvc = true,
#endif
    };

    m_logger = std::make_unique< shm::Logger >( std::move( logger_settings ) );

    auto log_create_err = m_logger->GetDirectoryCreationError();
    if ( !log_create_err.has_value() )
    {
        std::print( "Failed to create log directory: {}\n", log_create_err.error().message() );
        return false;
    }

    return true;
}
