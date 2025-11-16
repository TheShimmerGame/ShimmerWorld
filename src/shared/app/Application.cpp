
#include "Application.hpp"

#include <args.hxx>
#include <flecs.h>

#include <filesystem>
#include <iostream> // std::cerr, std::endl - in case we error out early and we do not have logging setup yet
#include <print>

#include "logging/Logging.hpp"

wb::Application::Application()
    : m_broker_world( std::make_unique< flecs::world >() )
{
}

wb::Application::~Application() = default;

// TODO move me to defines
constexpr uint32_t TARGET_FPS = 120;

int wb::Application::Run( int argc, char ** argv )
{
    // m_broker_world->app()
    //     .target_fps( TARGET_FPS )
    //     .run();

    std::filesystem::path config_directory = ".";
    args::ArgumentParser parser( "Shimmer World Broker Application",
                                 "A broker application for managing connections from players and service servers." );
    try
    {
        args::HelpFlag help( parser, "help", "Display this help menu", { 'h', "help" } );
        args::ValueFlag< std::string > config_dir( parser, "config-dir", "Directory to load configuration files from",
                                                   { 'c', "config-dir" }, args::Options::Single );
        // args supports completion in bash/zsh/fish shells via CompletionFlag, but we have no use for it at the moment
        // TODO: add it at some point, see https://github.com/Taywee/args/issues/126
        parser.ParseCLI( argc, argv );

        if ( config_dir )
            config_directory = config_dir.Get();
    }
    catch ( const args::Completion & e )
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

    // TODO: build path out of config
    // TODO: load config files
    // TODO: pass in the loggers name?
    //shm::InitLogging( {}, "worldbroker_log.txt" );
    //shm::Log( shm::Info, "Application starting {}", 1 );
    auto shm_sink = shm::Sink::CreateLogSink( shm::LoggingInitData{ "worldbroker.log", "./logs/", 1024 * 1024 * 5, 5 } );
    auto general_logger = shm_sink->AttachLogger( "worldbroker_general", spdlog::level::info, spdlog::level::info );
    general_logger->Log( spdlog::level::info, "Application starting {}", 1 );

    auto some_other_logger = shm_sink->AttachLogger( "worldbroker_other", spdlog::level::debug, spdlog::level::warn );
    some_other_logger->Log( spdlog::level::debug, "This is a debug message: {}", 42 );
    some_other_logger->Log( spdlog::level::info, "This is an info message: {}", 3.14 );
    some_other_logger->Log( spdlog::level::warn, "This is a warning message" );

    return 0;
}
