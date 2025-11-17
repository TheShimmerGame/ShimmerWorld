
#pragma once

#include <concepts>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

#include <rfl.hpp>
#include <rfl/json.hpp>

#include "filesystem/Filesystem.hpp"
#include "hash/Hash.hpp"
#include "logging/Logging.hpp"
#include "results/Result.hpp"

namespace shm
{
    template< typename C >
    concept IsConfigStructure = requires {
        { C::ConfigName } -> std::convertible_to< std::string_view >;
        { C::ConfigVersion } -> std::convertible_to< uint32_t >;
    };

    struct ConfigObject
    {
        template< IsConfigStructure Config >
        ConfigObject( Config && cfg )
            : m_impl( std::make_unique< ConfigImpl< Config > >( std::forward< Config >( cfg ) ) )
        {
        }

        struct IConfigImpl
        {
            virtual ~IConfigImpl() = default;

            [[nodiscard]] virtual shm::Result< void > LoadFromJson( const std::string & json_data ) = 0;
            [[nodiscard]] virtual shm::Result< void > SaveToJson( std::string & out_json_data )     = 0;
            virtual std::string_view GetConfigFileName()                                            = 0;
            virtual uint32_t GetConfigVersion() const                                               = 0;
            virtual std::type_index Type() const noexcept                                           = 0;
            [[nodiscard]] virtual void * DataPtr() noexcept                                         = 0;
            [[nodiscard]] virtual const void * DataPtr() const noexcept                             = 0;

            [[nodiscard]] virtual size_t ComputeHash() const = 0;

            template< IsConfigStructure Cfg, typename Self >
            [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
            {
                using ReturnType            = std::conditional_t< std::is_const_v< Self >, const Cfg *, Cfg * >;
                static ReturnType s_nullptr = nullptr;

                if ( self.Type() != std::type_index( typeid( Cfg ) ) )
                    return s_nullptr;

                if constexpr ( std::is_const_v< Self > )
                {
                    return static_cast< const Cfg * >( self.DataPtr() );
                }
                else
                {
                    return static_cast< Cfg * >( self.DataPtr() );
                }
            }
        };

        template< IsConfigStructure Config >
        struct ConfigImpl final : public IConfigImpl
        {
            ConfigImpl( Config && cfg )
                : m_config( std::forward< Config >( cfg ) )
            {
            }

            [[nodiscard]] shm::Result< void > SaveToJson( std::string & out_json_data ) override
            {
                try
                {
                    out_json_data = rfl::json::write( m_config, rfl::json::pretty );
                    return {};
                }
                catch ( const std::exception & )
                {
                    return std::unexpected( std::make_error_code( std::errc::io_error ) );
                }
            }

            [[nodiscard]] shm::Result< void > LoadFromJson( const std::string & json_data ) override
            {
                rfl::Result< Config > value = rfl::json::read< Config >( json_data );
                m_config                    = std::move( value.value_or( Config{} ) );
                // TODO: Actually handle the error
                return {};
            }

            [[nodiscard]] std::type_index Type() const noexcept override
            {
                return std::type_index( typeid( Config ) );
            }

            [[nodiscard]] void * DataPtr() noexcept override
            {
                return &m_config;
            }

            [[nodiscard]] const void * DataPtr() const noexcept override
            {
                return &m_config;
            }

            [[nodiscard]] size_t ComputeHash() const override
            {
                return shm::hash( m_config );
            }

            std::string_view GetConfigFileName() override
            {
                return Config::ConfigName;
            }

            uint32_t GetConfigVersion() const override
            {
                return Config::ConfigVersion;
            }

        protected:
            Config m_config;
        };

    public:
        template< IsConfigStructure Cfg, typename Self >
        [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
        {
            // If the caller requests mutable (non-const) access to a config instance of type `Cfg`,
            // mark this ConfigObject as "possibly dirty" by setting it's hash_at_get.
            // Config::Update() will check objects with non zero values in this
            // and if their current hash no longer matches `m_last_hash`, save them to disk
            // Const access does not change the dirty flag.
            if constexpr ( !std::is_const_v< Cfg > )
            {
                if ( self.Type() == std::type_index( typeid( Cfg ) ) )
                    self.m_hash_at_get = self.m_impl->ComputeHash();
            }

            return self.m_impl->template Get< Cfg >();
        }

        std::type_index Type() const noexcept;
        [[nodiscard]] bool IsPossiblyDirty() const noexcept;
        bool RefreshIfDirty();

    protected:
        // Grant access to Config so that it can call m_impl->Save/Load
        friend struct Config;

        std::unique_ptr< IConfigImpl > m_impl;

        /**
         * Stores the hash value of the configuration object at the time of the last mutable access.
         * Used to detect changes: if the current hash differs from this value during an update,
         * the configuration is considered modified and may be persisted (e.g., saved to disk).
         * Reset or updated on each mutable access via Get<>.
         */
        size_t m_hash_at_get = 0;
    };

    struct Config
    {
        Config( std::string_view log_root_dir, shm::Sink * );
        virtual ~Config();
        Config( const Config & )             = delete;
        Config( Config && )                  = delete;
        Config & operator=( const Config & ) = delete;
        Config & operator=( Config && )      = delete;

        template< IsConfigStructure Cfg, typename Self >
        [[nodiscard]] auto GetConfig( this Self & self ) -> decltype( auto )
        {
            using ConfigType                 = std::conditional_t< std::is_const_v< Self >, const Cfg, Cfg >;
            static ConfigType * s_non_config = nullptr;

            auto iter = std::ranges::find( self.m_configs, std::type_index( typeid( Cfg ) ), &ConfigObject::Type );
            if ( iter == self.m_configs.end() )
                return s_non_config;

            return ( *iter )->template Get< Cfg >();
        }

        template< IsConfigStructure... Args >
        void RegisterConfigs( Args &&... args )
        {
            ( RegisterConfig( std::forward< Args >( args ) ), ... );
        }

        bool Update();

    protected:
        template< IsConfigStructure Cfg >
        void RegisterConfig( Cfg && cfg )
        {
            auto json_data_res = shm::fs::ReadFileToString( fmt::format( "{}{}.json", m_log_root_dir, Cfg::ConfigName ) ).value_or( {} );
            auto obj           = std::make_unique< ConfigObject >( std::forward< Cfg >( cfg ) );
            auto & config_obj  = m_configs.emplace_back( std::move( obj ) );

            // TODO: Version migrators, each loaded config should contain a version field

            if ( !json_data_res.empty() )
            {
                auto load_result = config_obj->m_impl->LoadFromJson( json_data_res );
                if ( !load_result )
                {
                    m_logger->Log( spdlog::level::err, "Failed to load config object. Name: {}", Cfg::ConfigName );
            }
        }

    private:
        std::vector< std::unique_ptr< ConfigObject > > m_configs;
        std::unique_ptr< shm::Logger > m_logger = nullptr;
        std::string m_log_root_dir              = "./logs/";
    };
} // namespace shm
