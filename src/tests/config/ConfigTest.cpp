
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "config/Config.hpp"

#include <future>

struct TestConfig
{
    static constexpr uint32_t ConfigVersion = 1;

    int variable = 0;
};

static shm::Result< TestConfig > TestConfigMigrationFunction( std::string & /*json_data*/, uint32_t from_version, uint32_t to_version )
{
    TestConfig config;
    if ( from_version == 1 && to_version == 2 )
        INFO( "Migration from version 1 to 2" );

    return config;
}

static constexpr std::string_view G_TestConfigDir = "./Testing/Configs/";
namespace shm::cfg
{
    TEST_CASE( "shm::Config" )
    {
        SUBCASE( "Directory creation" )
        {
            SUBCASE( "Invalid Directory" )
            {
                shm::Config invalid_config_obj{ R"(./invalid_path/+$##?????:/)" };
                auto dir_result = invalid_config_obj.IsDirectoryCreated();
                CHECK( !dir_result.has_value() );
            }

            SUBCASE( "Valid Directory" )
            {
                shm::Config valid_config_obj{ G_TestConfigDir };
                auto dir_result = valid_config_obj.IsDirectoryCreated();
                CHECK( dir_result.has_value() );
            }
        }

        SUBCASE( "Get unregistered config" )
        {
            shm::Config config_obj{ G_TestConfigDir };
            auto test_config = config_obj.GetConfig< const TestConfig >( "NonExistentConfig" );
            CHECK( !test_config.IsValid() );
        }

        SUBCASE( "Register config/duplicate config" )
        {
            shm::Config config_obj{ G_TestConfigDir };
            auto reg_result = config_obj.RegisterConfig( TestConfig{}, "TestConfig", "json", &TestConfigMigrationFunction );
            CHECK( reg_result.has_value() );
        }

        SUBCASE( "Modyfing and retrieving config" )
        {
            shm::Config config_obj{ G_TestConfigDir };
            auto reg_result = config_obj.RegisterConfig( TestConfig{}, "TestConfig", "json", &TestConfigMigrationFunction );
            CHECK( reg_result.has_value() );

            // scope so that the accessor dies and writes back into config_obj
            {
                auto test_config_accessor = config_obj.GetConfig< TestConfig >( "TestConfig" );
                CHECK( test_config_accessor.IsValid() );
                test_config_accessor->variable = 42;
            }

            config_obj.SaveDirtyConfigs();

            auto test_config_accessor_2 = config_obj.GetConfig< const TestConfig >( "TestConfig" );
            CHECK( test_config_accessor_2.IsValid() );
            CHECK( test_config_accessor_2->variable == 42 );
        }
    }
} // namespace shm::cfg
