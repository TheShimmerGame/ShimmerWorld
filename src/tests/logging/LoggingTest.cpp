
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "filesystem/Filesystem.hpp"
#include "logging/Logging.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <spdlog/spdlog.h>
#include <thread>

namespace
{
    static constexpr std::string_view G_BaseTestLogDir = "./Testing/Logs/";

    shm::Result< void > CleanDir( const std::filesystem::path & p )
    {
        std::error_code ec;
        if ( std::filesystem::exists( p, ec ) )
            std::filesystem::remove_all( p, ec );

        if ( ec )
            return std::unexpected( ec );

        return {};
    }

    // Print absolute path on failure; no explicit spdlog reset needed as Logger::~Logger handles it
    void RequireCleanDir( const std::filesystem::path & p )
    {
        auto res = CleanDir( p );
        if ( !res.has_value() )
        {
            const auto abs = std::filesystem::absolute( p );
            INFO( fmt::format( "CleanDir failed for absolute path: {}", abs.string() ) );
            FAIL_CHECK( res.error().message() );
        }
    }

    // Per-subcase directory to avoid cross-subcase handle reuse
    std::filesystem::path SubcaseDir( std::string_view name )
    {
        return std::filesystem::path( G_BaseTestLogDir ) / name;
    }
} // namespace

namespace shm::log
{
    TEST_CASE( "shm::Logger" )
    {
        SUBCASE( "Directory creation failure" )
        {
            RequireCleanDir( SubcaseDir( "DirCreateFail" ) );
            shm::LoggerSettings s;
            s.m_log_file_path = R"(./invalid_path/+$##?????:/)";
            s.m_log_file_name = "ShouldNotExist.log";
            shm::Logger logger( s );
            auto dir_result = logger.GetDirectoryCreationError();
            CHECK( !dir_result.has_value() );
        }

        SUBCASE( "Directory creation success" )
        {
            const auto dir = SubcaseDir( "DirCreateSuccess" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_log_file_path = dir.string();
            s.m_log_file_name = "CreationSuccess.log";
            shm::Logger logger( s );
            auto dir_result = logger.GetDirectoryCreationError();
            CHECK( dir_result.has_value() );
            std::error_code ec{};
            const bool dir_exists = std::filesystem::exists( std::filesystem::path( s.m_log_file_path ), ec );
            CHECK( dir_exists );
        }

        SUBCASE( "Default logger name is applied" )
        {
            const auto dir = SubcaseDir( "DefaultLoggerName" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_logger_name   = "testlogger";
            s.m_log_file_path = dir.string();
            s.m_log_file_name = "DefaultLoggerName.log";
            shm::Logger logger( s );
            auto default_logger = spdlog::default_logger();
            REQUIRE( default_logger );
            CHECK( default_logger->name() == s.m_logger_name );
        }

        SUBCASE( "Scope nesting formatting" )
        {
            const auto dir = SubcaseDir( "ScopeNesting" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_log_file_path = dir.string();
            s.m_log_file_name = "ScopeNesting.log";
            s.m_logger_name   = "scopelogger";
            shm::Logger logger( s );

            {
                shm::LogScope outer( "Outer" );
                spdlog::info( "Message without inner scope" );
                {
                    shm::LogScope inner( "Inner" );
                    spdlog::info( "Message with inner scope" );
                }
                spdlog::info( "Message after inner scope destruction" );
            }

            spdlog::default_logger()->flush();

            const auto log_path = std::filesystem::path( s.m_log_file_path ) / s.m_log_file_name;
            auto content        = shm::fs::ReadFileToString( log_path );

            CHECK( ( content.has_value() && !content->empty() ) );
            CHECK( content->find( "[Outer]: Message without inner scope" ) != std::string::npos );
            CHECK( content->find( "[Outer::Inner]: Message with inner scope" ) != std::string::npos );
            CHECK( content->find( "[Outer]: Message after inner scope destruction" ) != std::string::npos );
        }

        SUBCASE( "Scopes are thread-local" )
        {
            const auto dir = SubcaseDir( "ThreadLocalScopes" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_log_file_path = dir.string();
            s.m_log_file_name = "ThreadLocalScopes.log";
            s.m_logger_name   = "threadscope";
            shm::Logger logger( s );

            auto worker = []( std::string name )
            {
                shm::LogScope scope( name );
                for ( int i = 0; i < 5; ++i )
                    spdlog::info( "Line {}", i );
            };

            {
                std::jthread t1( worker, "ThreadOne" );
                std::jthread t2( worker, "ThreadTwo" );
            }

            spdlog::default_logger()->flush();

            const auto log_path = std::filesystem::path( s.m_log_file_path ) / s.m_log_file_name;
            auto content        = shm::fs::ReadFileToString( log_path );

            CHECK( ( content.has_value() && !content->empty() ) );
            CHECK( content->find( "[ThreadOne]:" ) != std::string::npos );
            CHECK( content->find( "[ThreadTwo]:" ) != std::string::npos );
            CHECK( content->find( "[ThreadOne::ThreadTwo]:" ) == std::string::npos );
            CHECK( content->find( "[ThreadTwo::ThreadOne]:" ) == std::string::npos );
        }

        SUBCASE( "Flush level behavior" )
        {
            const auto dir = SubcaseDir( "FlushLevel" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_log_file_path = dir.string();
            s.m_log_file_name = "FlushLevel.log";
            s.m_logger_name   = "flushlogger";
            s.m_flush_level   = spdlog::level::info;
            s.m_level         = spdlog::level::trace;
            shm::Logger logger( s );
            spdlog::trace( "Trace message (might not flush immediately)" );
            spdlog::info( "Info message triggers flush" );
            spdlog::default_logger()->flush();

            const auto log_path = std::filesystem::path( s.m_log_file_path ) / s.m_log_file_name;
            auto content        = shm::fs::ReadFileToString( log_path );
            CHECK( ( content.has_value() && !content->empty() ) );
            CHECK( content->find( "Info message triggers flush" ) != std::string::npos );
        }

        SUBCASE( "File rotation" )
        {
            const auto dir = SubcaseDir( "Rotation" );
            RequireCleanDir( dir );
            shm::LoggerSettings s;
            s.m_log_file_path       = dir.string();
            s.m_log_file_name       = "RotationTest.log";
            s.m_logger_name         = "rotationlogger";
            s.m_max_file_size_bytes = 64; // keep tiny to force rotation quickly
            s.m_max_files           = 3;
            shm::Logger logger( s );

            for ( int i = 0; i < 2000; ++i )
            {
                spdlog::info( "Rotating line number {} -- {}", i,
                              "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" );
            }
            spdlog::default_logger()->flush();

            const auto base_path = std::filesystem::path( s.m_log_file_path );
            const auto main_file = base_path / s.m_log_file_name;
            CHECK( std::filesystem::exists( main_file ) );

            bool rotated_found = false;
            std::error_code ec;
            for ( const auto & entry : std::filesystem::directory_iterator( base_path, ec ) )
            {
                const std::string file_name = entry.path().filename().string();
                if ( file_name.starts_with( s.m_log_file_name ) )
                    rotated_found = true;
            }

            if ( ec )
            {
                const auto abs = std::filesystem::absolute( base_path );
                INFO( fmt::format( "Directory iteration failed for absolute path: {}", abs.string() ) );
                FAIL_CHECK( ec.message() );
            }

            CHECK( rotated_found );
        }

        SUBCASE( "Directory without ending slash" )
        {
            const auto dir_no_slash = SubcaseDir( "NoEndingSlash" );
            RequireCleanDir( dir_no_slash );
            std::string path_no_slash = dir_no_slash.string(); // no trailing slash on purpose

            shm::LoggerSettings s;
            s.m_log_file_path = path_no_slash;
            s.m_log_file_name = "NoEndingSlash.log";
            s.m_logger_name   = "noslashlogger";

            shm::Logger logger( s );

            auto dir_result = logger.GetDirectoryCreationError();
            CHECK( dir_result.has_value() );

            // emit a line and ensure the file exists and is readable
            spdlog::info( "Directory path without trailing slash should work" );
            spdlog::default_logger()->flush();

            const auto log_path = std::filesystem::path( s.m_log_file_path ) / s.m_log_file_name;
            CHECK( std::filesystem::exists( log_path ) );

            auto content = shm::fs::ReadFileToString( log_path );
            CHECK( ( content.has_value() && !content->empty() ) );
        }

        SUBCASE( "Directory with ending slash" )
        {
            const auto dir_with_slash = SubcaseDir( "WithEndingSlash//" );
            RequireCleanDir( dir_with_slash );

            shm::LoggerSettings s;
            s.m_log_file_path = dir_with_slash.string(); // ensure trailing slash
            s.m_log_file_name = "WithEndingSlash.log";
            s.m_logger_name   = "withslashlogger";

            shm::Logger logger( s );

            auto dir_result = logger.GetDirectoryCreationError();
            CHECK( dir_result.has_value() );

            spdlog::info( "Directory path with trailing slash should work" );
            spdlog::default_logger()->flush();

            const auto log_path = std::filesystem::path( s.m_log_file_path ) / s.m_log_file_name;
            CHECK( std::filesystem::exists( log_path ) );

            auto content = shm::fs::ReadFileToString( log_path );
            CHECK( ( content.has_value() && !content->empty() ) );
        }
    }
} // namespace shm::log
