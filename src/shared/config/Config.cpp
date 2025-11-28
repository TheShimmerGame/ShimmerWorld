
#include "Config.hpp"
#include "filesystem/Filesystem.hpp"

#include <spdlog/spdlog.h>

shm::Config::~Config()
{
    // TODO: guard this behind a feature flag
    //SaveDirtyConfigs();
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

std::vector< shm::Result< void > > shm::Config::SaveDirtyConfigs()
{
    std::vector< shm::Result< void > > results;
    for ( auto & cfg_obj : m_configs )
    {
        if ( !cfg_obj )
            continue;

        auto pending_result = cfg_obj->m_impl->ApplyPendingUpdate();
        if ( !pending_result.has_value() )
        {
            auto msg = pending_result.error().message();
            results.emplace_back( pending_result );
            continue;
        }

        results.emplace_back( shm::Result< void >{} );
    }
    return results;
}

/** CONFIG OBJECT **/

std::type_index shm::ConfigObject::Type() const noexcept
{
    return m_impl->Type();
}
