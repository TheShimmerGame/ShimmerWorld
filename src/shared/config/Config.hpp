//
// #pragma once
//
// #include <concepts>
// #include <cstdio>
// #include <functional>
// #include <memory>
// #include <mutex>
// #include <string>
// #include <type_traits>
// #include <typeindex>
// #include <utility>
// #include <vector>
//
// #include <fmt/format.h>
// #include <rfl.hpp>
// #include <rfl/json.hpp>
//
// #include "filesystem/Filesystem.hpp"
// #include "results/Result.hpp"
//
// namespace shm
//{
//    template< typename C >
//    concept IsConfigStructure = requires {
//        { C::ConfigVersion } -> std::convertible_to< uint32_t >;
//    };
//
//    template< typename Fn, typename F >
//    concept IsVersionMigratorFnPtr =
//        std::is_same_v< Fn, shm::Result< F > ( * )( std::string &, uint32_t, uint32_t ) >;
//
//    struct ConfigObject
//    {
//        friend struct Config;
//
//        template< IsConfigStructure Config >
//        ConfigObject( Config && cfg, std::string_view config_name )
//            : m_impl( std::make_unique< ConfigImpl< Config > >( std::forward< Config >( cfg ), config_name ) )
//        {
//        }
//
//    protected:
//        struct IConfig
//        {
//            virtual ~IConfig() = default;
//
//            [[nodiscard]] virtual shm::Result< void > LoadFromJson( const std::string & json_data ) = 0;
//            [[nodiscard]] virtual shm::Result< void > SaveToJson( std::string & out_json_data )     = 0;
//            virtual std::string_view GetConfigFileName()                                            = 0;
//            virtual uint32_t GetConfigVersion() const                                               = 0;
//            virtual std::type_index Type() const noexcept                                           = 0;
//            [[nodiscard]] virtual void * DataPtr() noexcept                                         = 0;
//            [[nodiscard]] virtual const void * DataPtr() const noexcept                             = 0;
//
//            template< IsConfigStructure Cfg, typename Self >
//            [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
//            {
//                std::scoped_lock lock( self.m_access_lock );
//                // TODO: Self might be const or non-const as well
//                using ReturnType            = std::conditional_t< std::is_const_v< Cfg >, const Cfg *, Cfg * >;
//                static ReturnType s_nullptr = nullptr;
//
//                if ( self.Type() != std::type_index( typeid( Cfg ) ) )
//                    return s_nullptr;
//
//                return static_cast< ReturnType >( self.DataPtr() );
//            }
//
//
//        template< IsConfigStructure Config >
//        struct ConfigImpl final : public IConfig
//        {
//            ConfigImpl( Config && cfg, std::string_view cfg_name )
//                : m_config( std::forward< Config >( cfg ) )
//                , m_config_name( cfg_name )
//            {
//            }
//
//            [[nodiscard]] shm::Result< void > SaveToJson( std::string & out_json_data ) override
//            {
//                try
//                {
//                    out_json_data = rfl::json::write( m_config, rfl::json::pretty );
//                    return {};
//                }
//                catch ( const std::exception & )
//                {
//                    return std::unexpected( std::make_error_code( std::errc::io_error ) );
//                }
//            }
//
//            [[nodiscard]] shm::Result< void > LoadFromJson( const std::string & json_data ) override
//            {
//                rfl::Result< Config > value = rfl::json::read< Config >( json_data );
//                m_config                    = std::move( value.value_or( Config{} ) );
//                // TODO: Actually handle the error
//                return {};
//            }
//
//            [[nodiscard]] std::type_index Type() const noexcept override
//            {
//                return std::type_index( typeid( Config ) );
//            }
//
//            [[nodiscard]] void * DataPtr() noexcept override
//            {
//                return &m_config;
//            }
//
//            [[nodiscard]] const void * DataPtr() const noexcept override
//            {
//                return &m_config;
//            }
//
//            std::string_view GetConfigFileName() override
//            {
//                return m_config_name;
//            }
//
//            uint32_t GetConfigVersion() const override
//            {
//                return Config::ConfigVersion;
//            }
//
//        protected:
//            Config m_config;
//            std::string_view m_config_name;
//            std::unique_ptr< Config > m_config_update = nullptr;
//        };
//
//        template< IsConfigStructure Cfg, typename Self >
//        [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
//        {
//            return self.m_impl->template Get< Cfg >();
//        }
//
//        std::type_index Type() const noexcept;
//
//    private:
//        std::unique_ptr< IConfig > m_impl;
//    };
//
//    template< IsConfigStructure Cfg >
//    struct ConfigAccessor
//    {
//        ConfigAccessor() = default;
//
//        ConfigAccessor( const ConfigAccessor & )             = delete;
//        ConfigAccessor & operator=( const ConfigAccessor & ) = delete;
//
//        ConfigAccessor( ConfigAccessor && )             = default;
//        ConfigAccessor & operator=( ConfigAccessor && ) = default;
//
//        explicit ConfigAccessor( ConfigObject * config_object,
//                                 Config * config_reg,
//                                 Cfg cfg_copy,
//                                 std::string_view config_name ) noexcept
//            : m_config_object( config_object )
//            , m_config_reg( config_reg )
//            , m_cfg_copy( std::forward< Cfg >( cfg_copy ) )
//            , m_config_name( config_name )
//        {
//        }
//
//        /// @brief On destruction, if the config was modified, mark it as dirty in the parent Config object.
//        /// This will cause it to be saved back to disk.
//        /// Declared here, definition below after Config to avoid circular dependency
//        ~ConfigAccessor();
//
//        [[nodiscard]] bool IsValid() const noexcept
//        {
//            return m_config_object != nullptr;
//        }
//
//        explicit operator bool() const noexcept
//        {
//            return IsValid();
//        }
//
//        auto operator->() -> Cfg *
//        {
//            using ReturnType                = std::conditional_t< std::is_const_v< Cfg >, const Cfg *, Cfg * >;
//            static ReturnType no_result_cfg = nullptr;
//            if ( !m_config_object || !m_config_reg )
//                return no_result_cfg;
//
//            m_needs_synchro = !std::is_const_v< Cfg >;
//            return &m_cfg_copy;
//        }
//
//    protected:
//        friend struct Config;
//
//        ConfigObject * m_config_object = nullptr;
//        Config * m_config_reg          = nullptr;
//        Cfg m_cfg_copy                 = {};
//        std::string_view m_config_name = "";
//        bool m_needs_synchro           = false;
//    };
//
//    struct Config
//    {
//        /// @brief Allow config accessor to call MarkAsDirty which is protected.
//        template< IsConfigStructure Cfg >
//        friend struct ConfigAccessor;
//
//        Config( std::string_view log_root_dir );
//        virtual ~Config();
//        Config( const Config & )             = delete;
//        Config( Config && )                  = delete;
//        Config & operator=( const Config & ) = delete;
//        Config & operator=( Config && )      = delete;
//
//        /// @brief Indicates whether the config directory has been created (in ctor).
//        /// @return true if the directory has been created; otherwise false.
//        shm::Result< void > IsDirectoryCreated() const;
//
//        template< IsConfigStructure Cfg, typename Self >
//        [[nodiscard]] auto GetConfig( this Self & self, std::string_view config_name ) -> decltype( auto )
//        {
//            using ConfigType = std::conditional_t< std::is_const_v< Self >, const Cfg, Cfg >;
//
//            //// Disallow write access from non-main threads, so we do not have to lock here
//            // if constexpr ( !std::is_const_v< Self > )
//            //{
//            //     if ( self.m_main_thread_id != std::this_thread::get_id() )
//            //         return ConfigAccessor< ConfigType >{};
//            // }
//
//            auto iter = std::ranges::find_if( self.m_configs,
//                                              [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
//                                              {
//                                                  const auto cfg_type_ind = std::type_index( typeid( Cfg ) );
//                                                  return obj->m_impl->Type() == cfg_type_ind
//                                                         && obj->m_impl->GetConfigFileName() == config_name;
//                                              } );
//
//            if ( iter == self.m_configs.end() )
//                return ConfigAccessor< ConfigType >{};
//
//            auto cfg_ptr = ( *iter )->m_impl->template Get< ConfigType >();
//            return ConfigAccessor< ConfigType >{
//                iter->get(),
//                &self,
//                *cfg_ptr,
//                config_name };
//        }
//
//        template< IsConfigStructure Cfg, typename CfgMigratorFn >
//            requires IsVersionMigratorFnPtr< CfgMigratorFn, Cfg >
//        shm::Result< void > RegisterConfig( Cfg && cfg,
//                                            std::string_view config_name,
//                                            std::string_view config_extension,
//                                            CfgMigratorFn migrator )
//        {
//            // Config might already exist with given type and name
//            auto config_iter = std::ranges::find_if( m_configs,
//                                                     [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
//                                                     {
//                                                         return obj->m_impl->Type() == std::type_index( typeid( Cfg ) ) && obj->m_impl->GetConfigFileName() == config_name;
//                                                     } );
//            if ( config_iter != m_configs.end() )
//                return std::unexpected( std::make_error_code( std::errc::file_exists ) );
//
//            auto obj          = std::make_unique< ConfigObject >( std::forward< Cfg >( cfg ), config_name );
//            auto & config_obj = m_configs.emplace_back( std::move( obj ) );
//
//            auto json_data_res = shm::fs::ReadFileToString( fmt::format( "{}{}.{}", m_log_root_dir, config_name, config_extension ) ).value_or( {} );
//            // TODO: Version migrators, each loaded config should contain a version field
//            if ( !json_data_res.empty() )
//            {
//                auto load_result = config_obj->m_impl->LoadFromJson( json_data_res );
//                if ( !load_result.has_value() )
//                    return load_result;
//            }
//
//            return {};
//        }
//
//        void SaveDirtyConfigs();
//
//    protected:
//        template< IsConfigStructure Cfg >
//        void MarkAsDirty( Cfg && possibly_new_data, std::string_view config_name )
//        {
//            auto config_iter = std::ranges::find_if( m_configs,
//                                                     [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
//                                                     {
//                                                         return obj->m_impl->Type() == std::type_index( typeid( Cfg ) ) && obj->m_impl->GetConfigFileName() == config_name;
//                                                     } );
//            if ( config_iter == m_configs.end() )
//                return;
//
//            ( *config_iter )->m_impl->SetPendingUpdate( &possibly_new_data );
//        }
//
//    private:
//        // Assumed to be stable after initial creation (in main) so we do not have to synchronize access to it
//        // but we have to synchronize to the underlying data
//        std::vector< std::unique_ptr< ConfigObject > > m_configs;
//        std::string m_log_root_dir = "./logs/";
//        /// @brief Possible error during creation of config
//        std::error_code m_directory_err{};
//    };
//
//    template< IsConfigStructure Cfg >
//    ConfigAccessor< Cfg >::~ConfigAccessor()
//    {
//        if constexpr ( !std::is_const_v< Cfg > )
//        {
//            if ( m_needs_synchro && m_config_object && m_config_reg )
//                m_config_reg->MarkAsDirty( std::move( m_cfg_copy ), m_config_name );
//        }
//    }
//} // namespace shm
//
#pragma once

#include <concepts>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <rfl.hpp>
#include <rfl/json.hpp>

#include "filesystem/Filesystem.hpp"
#include "results/Result.hpp"

namespace shm
{
    template< typename C >
    concept IsConfigStructure = requires {
        { C::ConfigVersion } -> std::convertible_to< uint32_t >;
    };

    template< typename Fn, typename F >
    concept IsVersionMigratorFnPtr =
        std::is_same_v< Fn, shm::Result< F > ( * )( std::string &, uint32_t, uint32_t ) >;

    struct ConfigObject
    {
        friend struct Config;

        template< IsConfigStructure ConfigT >
        ConfigObject( ConfigT && cfg, std::string_view config_name )
            : m_impl( std::make_unique< ConfigImpl< ConfigT > >( std::forward< ConfigT >( cfg ), config_name ) )
        {
        }

    protected:
        struct IConfig
        {
            virtual ~IConfig() = default;

            [[nodiscard]] virtual shm::Result< void > LoadFromJson( const std::string & json_data ) = 0;
            virtual std::string_view GetConfigFileName()                                            = 0;
            virtual uint32_t GetConfigVersion() const                                               = 0;
            virtual std::type_index Type() const noexcept                                           = 0;
            [[nodiscard]] virtual void * DataPtr() noexcept                                         = 0;
            [[nodiscard]] virtual const void * DataPtr() const noexcept                             = 0;
            virtual void SetPendingUpdate( void * new_data )                                        = 0;
            virtual shm::Result< void > ApplyPendingUpdate()                                        = 0;

            template< IsConfigStructure Cfg, typename Self >
            [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
            {
                std::scoped_lock lock( self.m_access_lock );
                using ReturnType            = std::conditional_t< std::is_const_v< Cfg >, const Cfg *, Cfg * >;
                static ReturnType s_nullptr = nullptr;

                if ( self.Type() != std::type_index( typeid( Cfg ) ) )
                    return s_nullptr;

                return static_cast< ReturnType >( self.DataPtr() );
            }

            mutable std::mutex m_access_lock;
        };

        template< IsConfigStructure ConfigT >
        struct ConfigImpl final : public IConfig
        {
            ConfigImpl( ConfigT && cfg, std::string_view cfg_name )
                : m_config( std::forward< ConfigT >( cfg ) )
                , m_config_name( cfg_name )
            {
            }

            [[nodiscard]] shm::Result< void > LoadFromJson( const std::string & json_data ) override
            {
                rfl::Result< ConfigT > value = rfl::json::read< ConfigT >( json_data );
                std::scoped_lock lock( m_access_lock );
                m_config = std::move( value.value_or( ConfigT{} ) );
                return {};
            }

            [[nodiscard]] std::type_index Type() const noexcept override
            {
                return std::type_index( typeid( ConfigT ) );
            }

            [[nodiscard]] void * DataPtr() noexcept override
            {
                return &m_config;
            }

            [[nodiscard]] const void * DataPtr() const noexcept override
            {
                return &m_config;
            }

            std::string_view GetConfigFileName() override
            {
                return m_config_name;
            }

            uint32_t GetConfigVersion() const override
            {
                return ConfigT::ConfigVersion;
            }

            void SetPendingUpdate( void * new_data ) override
            {
                if ( !new_data )
                    return;

                std::scoped_lock lock( m_access_lock );
                if ( !m_config_update )
                    m_config_update = std::make_unique< ConfigT >( *static_cast< ConfigT * >( new_data ) );
                else
                    *m_config_update = *static_cast< ConfigT * >( new_data );
            }

            shm::Result< void > ApplyPendingUpdate() override
            {
                std::scoped_lock lock( m_access_lock );
                if ( !m_config_update )
                    return {};

                m_config        = std::move( *m_config_update );
                m_config_update = nullptr;

                try
                {

                    std::string json_data = rfl::json::write( m_config, rfl::json::pretty );
                    const auto path = fmt::format( "{}{}.json", "./logs/", m_config_name );
                    auto write_result     = shm::fs::WriteStringToFile( path, json_data, std::ios::in | std::ios::trunc );
                    if ( !write_result.has_value() )
                        return write_result;
                }
                catch ( const std::exception & )
                {
                    return std::unexpected( std::make_error_code( std::errc::io_error ) );
                }

                return {};
            }

        protected:
            ConfigT m_config;
            std::string_view m_config_name;
            std::unique_ptr< ConfigT > m_config_update = nullptr;
        };

        template< IsConfigStructure Cfg, typename Self >
        [[nodiscard]] auto Get( this Self & self ) -> decltype( auto )
        {
            return self.m_impl->template Get< Cfg >();
        }

        std::type_index Type() const noexcept;

    private:
        std::unique_ptr< IConfig > m_impl;
    };

    template< IsConfigStructure Cfg >
    struct ConfigAccessor
    {
        ConfigAccessor() = default;

        ConfigAccessor( const ConfigAccessor & )             = delete;
        ConfigAccessor & operator=( const ConfigAccessor & ) = delete;

        ConfigAccessor( ConfigAccessor && )             = default;
        ConfigAccessor & operator=( ConfigAccessor && ) = default;

        explicit ConfigAccessor( ConfigObject * config_object,
                                 Cfg cfg_copy,
                                 std::string_view config_name ) noexcept
            : m_config_object( config_object )
            , m_cfg_copy( std::forward< Cfg >( cfg_copy ) )
            , m_config_name( config_name )
        {
        }

        ~ConfigAccessor();

        [[nodiscard]] bool IsValid() const noexcept
        {
            return m_config_object != nullptr;
        }

        explicit operator bool() const noexcept
        {
            return IsValid();
        }

        auto operator->() -> Cfg *
        {
            using ReturnType                = std::conditional_t< std::is_const_v< Cfg >, const Cfg *, Cfg * >;
            static ReturnType no_result_cfg = nullptr;
            if ( !m_config_object )
                return no_result_cfg;

            m_needs_synchro = !std::is_const_v< Cfg >;
            return &m_cfg_copy;
        }

    protected:
        friend struct Config;

        ConfigObject * m_config_object = nullptr;
        Cfg m_cfg_copy                 = {};
        std::string_view m_config_name = "";
        bool m_needs_synchro           = false;
    };

    struct Config
    {
        template< IsConfigStructure Cfg >
        friend struct ConfigAccessor;

        Config( std::string_view log_root_dir );
        virtual ~Config();
        Config( const Config & )             = delete;
        Config( Config && )                  = delete;
        Config & operator=( const Config & ) = delete;
        Config & operator=( Config && )      = delete;

        shm::Result< void > IsDirectoryCreated() const;

        template< IsConfigStructure Cfg, typename Self >
        [[nodiscard]] auto GetConfig( this Self & self, std::string_view config_name ) -> decltype( auto )
        {
            using ConfigType = std::conditional_t< std::is_const_v< Self >, const Cfg, Cfg >;

            auto iter = std::ranges::find_if( self.m_configs,
                                              [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
                                              {
                                                  const auto cfg_type_ind = std::type_index( typeid( Cfg ) );
                                                  return obj->Type() == cfg_type_ind
                                                         && obj->m_impl->GetConfigFileName() == config_name;
                                              } );

            if ( iter == self.m_configs.end() )
                return ConfigAccessor< ConfigType >{};

            auto cfg_ptr = ( *iter )->m_impl->template Get< ConfigType >();
            return ConfigAccessor< ConfigType >{
                iter->get(),
                *cfg_ptr,
                config_name };
        }

        template< IsConfigStructure Cfg, typename CfgMigratorFn >
            requires IsVersionMigratorFnPtr< CfgMigratorFn, Cfg >
        shm::Result< void > RegisterConfig( Cfg && cfg,
                                            std::string_view config_name,
                                            std::string_view config_extension,
                                            CfgMigratorFn /*migrator*/ )
        {
            auto config_iter = std::ranges::find_if( m_configs,
                                                     [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
                                                     {
                                                         return obj->Type() == std::type_index( typeid( Cfg ) ) && obj->m_impl->GetConfigFileName() == config_name;
                                                     } );
            if ( config_iter != m_configs.end() )
                return std::unexpected( std::make_error_code( std::errc::file_exists ) );

            auto obj          = std::make_unique< ConfigObject >( std::forward< Cfg >( cfg ), config_name );
            auto & config_obj = m_configs.emplace_back( std::move( obj ) );

            auto json_data_res =
                shm::fs::ReadFileToString( fmt::format( "{}{}.{}", m_log_root_dir, config_name, config_extension ) )
                    .value_or( {} );
            if ( !json_data_res.empty() )
            {
                auto load_result = config_obj->m_impl->LoadFromJson( json_data_res );
                if ( !load_result.has_value() )
                    return load_result;
            }

            return {};
        }

        /// @brief Apply queued updates & persist each modified config to disk.
        /// Call from main thread.
        /// @return Num of configs that were dirty and were saved.
        size_t SaveDirtyConfigs();

    protected:
        template< IsConfigStructure Cfg >
        void MarkAsDirty( Cfg && possibly_new_data, std::string_view config_name )
        {
            auto config_iter = std::ranges::find_if( m_configs,
                                                     [ config_name ]( const std::unique_ptr< ConfigObject > & obj )
                                                     {
                                                         return obj->Type() == std::type_index( typeid( Cfg ) ) && obj->m_impl->GetConfigFileName() == config_name;
                                                     } );
            if ( config_iter == m_configs.end() )
                return;

            ( *config_iter )->m_impl->SetPendingUpdate( &possibly_new_data );
        }

    private:
        std::vector< std::unique_ptr< ConfigObject > > m_configs;
        std::string m_log_root_dir = "./logs/";
        std::error_code m_directory_err{};
    };

    template< IsConfigStructure Cfg >
    ConfigAccessor< Cfg >::~ConfigAccessor()
    {
        if constexpr ( !std::is_const_v< Cfg > )
        {
            //if ( m_needs_synchro && m_config_object )
                //m_config_reg->MarkAsDirty( std::move( m_cfg_copy ), m_config_name );
        }
    }

} // namespace shm