
#include "Config.hpp"
#include "ConfigComponents.hpp"
#include "DeSer/FlecsVector.hpp"
#include "logging/Logging.hpp"

#include <flecs.h>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>

namespace wb
{
    void CreateConfigDirectoryAndEnsurePaths( const shm::ConfigPathComponent & config_path_cmp )
    {
        if ( std::filesystem::exists( config_path_cmp.m_config_path ) )
            return;

        std::filesystem::path actual_path = config_path_cmp.m_config_path;
        // the provided path might be empty, in which case we want it relative to current working directory ( so, a . )
        if ( actual_path.empty() )
            actual_path = ".";

        if ( actual_path.has_filename() && actual_path.has_extension() )
        {
            printf( "Config path '%s' is expected to be a directory path, but appears to be a file path.\n",
                    actual_path.string().c_str() );
            std::abort();
        }

        if ( actual_path.has_relative_path() )
            std::filesystem::create_directories( std::filesystem::current_path() / actual_path );
        else
            std::filesystem::create_directories( actual_path );
    }

    void FillInitialConfig( shm::LoggingConfigComponent & config )
    {
        magic_enum::enum_for_each< shm::LogCategory >( [ & ]( auto category )
                                                       {
                                                           config.m_category_levels.emplace_back( static_cast< shm::LogCategory >( category ), shm::LogLevel::Debug );
                                                       } );
    }

    // Generic config loader: load if present; otherwise prefill (if any), then serialize and create the file.
    template< typename TConfig, typename PrefillFn >
    void LoadOrCreateConfig( flecs::world & world, const std::filesystem::path & cfg_path, PrefillFn prefill )
    {
        std::ifstream file( cfg_path, std::ios::in );
        if ( file.is_open() )
        {
            std::string json_content( ( std::istreambuf_iterator< char >( file ) ),
                                      std::istreambuf_iterator< char >() );

            TConfig & cfg = world.get_mut< TConfig >();
            world.from_json( &cfg, json_content.c_str() );
            return;
        }

        if ( std::filesystem::exists( cfg_path ) )
        {
            printf( "Failed to open config file at '%s'\n", cfg_path.string().c_str() );
            std::abort();
        }

        // File missing: construct from defaults in ECS world, optionally prefill, then persist to disk.
        TConfig & cfg = world.get_mut< TConfig >();
        prefill( cfg );
        auto serialized_cfg = world.to_json( &cfg );

        std::ofstream outfile( cfg_path, std::ios::out | std::ios::trunc );
        if ( outfile.is_open() )
        {
            outfile << serialized_cfg;
            outfile.close();
        }
        else
        {
            printf( "Failed to create config file at '%s'\n", cfg_path.string().c_str() );
            std::abort();
        }
    }

    // Overload for when no prefill is needed.
    template< typename TConfig >
    void LoadOrCreateConfig( flecs::world & world, const std::filesystem::path & cfg_path )
    {
        LoadOrCreateConfig< TConfig >( world, cfg_path, []( TConfig & )
                                       {
                                       } );
    }

    ConfigSystem::ConfigSystem( flecs::world & world )
    {
        /// Register components with reflection system
        world.component< shm::NetworkConfigComponent >()
            .member< uint32_t >( "temporary" )
            .add( flecs::Singleton );
        world.set< shm::NetworkConfigComponent >( shm::NetworkConfigComponent{} );

        world.component< shm::DatabaseConfigComponent >()
            .member< uint32_t >( "temporary" )
            .add( flecs::Singleton );
        world.set< shm::DatabaseConfigComponent >( shm::DatabaseConfigComponent{} );

        world.component< shm::CategoryLogLevel >()
            .member< shm::LogCategory >( "m_category" )
            .member< shm::LogLevel >( "m_level" );

        world.component< std::vector< shm::CategoryLogLevel > >()
            .opaque( shm::deser::fl::vector_deser< shm::CategoryLogLevel > );

        world.component< shm::LoggingConfigComponent >()
            .member< std::vector< shm::CategoryLogLevel > >( "m_category_levels" )
            .add( flecs::Singleton );
        world.set< shm::LoggingConfigComponent >( shm::LoggingConfigComponent{} );

        // Load in the config data from the disk, if it exists. The ConfigPathComponent has been set in Application
        const auto & config_path_cmp = world.get< shm::ConfigPathComponent >();
        auto logging_cfg_path        = config_path_cmp.m_config_path / "logging_config.json";
        auto network_cfg_path        = config_path_cmp.m_config_path / "network_config.json";
        auto database_cfg_path       = config_path_cmp.m_config_path / "database_config.json";
        // before we begin the load, user might have specified directory that does not exist, we need to create it then
        CreateConfigDirectoryAndEnsurePaths( config_path_cmp );

        // Load/create Logging config (with prefill)
        LoadOrCreateConfig< shm::LoggingConfigComponent >( world, logging_cfg_path, FillInitialConfig );

        // Load/create Network and Database configs (no prefill required)
        LoadOrCreateConfig< shm::NetworkConfigComponent >( world, network_cfg_path );
        LoadOrCreateConfig< shm::DatabaseConfigComponent >( world, database_cfg_path );
    }
} // namespace wb