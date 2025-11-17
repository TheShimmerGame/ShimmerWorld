
#include "Config.hpp"
#include "filesystem/Filesystem.hpp"

shm::Config::~Config() = default;

bool shm::Config::Update()
{
    bool any_changed = false;
    for ( auto & cfg_obj : m_configs )
    {
        if ( cfg_obj->IsPossiblyDirty() && cfg_obj->RefreshIfDirty() )
        {
            any_changed = true;

            std::string json_data;
            auto save_result = cfg_obj->m_impl->SaveToJson( json_data );
            if ( !save_result )
            {
                m_logger->Log( spdlog::level::err, "Failed to save config object of type {}: {}", cfg_obj->Type().name(),
                               save_result.error().message() );
                continue;
            }

            m_logger->Log( spdlog::level::info, "Config object of type {} has changed, saving to disk.", cfg_obj->Type().name() );
            std::filesystem::path config_path = fmt::format( "{}{}.json", m_log_root_dir, cfg_obj->m_impl->GetConfigFileName() );
            auto create_dir_res = shm::fs::CreateDirectories( std::filesystem::path( config_path ).parent_path() );
            if ( !create_dir_res )
            {
                m_logger->Log( spdlog::level::err, "Failed to create directories for config file {}: {}", config_path.string(), create_dir_res.error().message() );
                continue;
            }

            // TODO: Version migrators, each loaded config should contain a version field. We should append it here
            
            auto write_result = shm::fs::WriteStringToFile( config_path, json_data, std::ios::trunc | std::ios::out );
            if ( !write_result )
                m_logger->Log( spdlog::level::err, "Failed to write config file for type {}: {}", cfg_obj->Type().name(),
                               write_result.error().message() );
        }
    }
    return any_changed;
}

shm::Config::Config( std::string_view log_root_dir, shm::Sink * sink_ )
    : m_log_root_dir( log_root_dir )
    , m_logger( sink_->AttachLogger( "Config", spdlog::level::info, spdlog::level::info ) )
{
}

bool shm::ConfigObject::RefreshIfDirty()
{
    if ( !m_hash_at_get )
        return false;

    const size_t new_hash = m_impl->ComputeHash();
    const bool changed    = ( new_hash != m_hash_at_get );
    m_hash_at_get           = 0;
    return changed;
}

bool shm::ConfigObject::IsPossiblyDirty() const noexcept
{
    return m_hash_at_get != 0;
}

std::type_index shm::ConfigObject::Type() const noexcept
{
    return m_impl->Type();
}
