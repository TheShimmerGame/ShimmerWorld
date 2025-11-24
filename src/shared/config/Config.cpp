
#include "Config.hpp"
#include "filesystem/Filesystem.hpp"

#include <spdlog/spdlog.h>

shm::Config::~Config()
{
    SaveDirtyConfigs();
}

shm::Config::Config( std::string_view log_root_dir )
    : m_log_root_dir( log_root_dir )
{
    /// Create the root directory if it doesn't exist
    std::filesystem::create_directories( m_log_root_dir, m_directory_err );
}


shm::Result< void > shm::Config::IsDirectoryCreated() const
{
    if ( m_directory_err )
        return std::unexpected( m_directory_err );

    return {};
}

size_t shm::Config::SaveDirtyConfigs()
{
    for ( auto & cfg_obj : m_configs )
    {
        if ( !cfg_obj )
            continue;

        auto pending_result = cfg_obj->m_impl->ApplyPendingUpdate();
        if ( !pending_result.has_value() )
            continue; // Optional: log error
        // std::string json;
        // auto res = cfg_obj->m_impl->SaveToJson( json );
        // if ( !res.has_value() )
        //     continue; // Optional: log error
        //
        // const auto path =
        //     fmt::format( "{}{}.json", m_log_root_dir, cfg_obj->m_impl->GetConfigFileName() );
        //  shm::fs::WriteFileFromString( path, json );
    }
    return {};
}

/** CONFIG OBJECT **/

std::type_index shm::ConfigObject::Type() const noexcept
{
    return m_impl->Type();
}
