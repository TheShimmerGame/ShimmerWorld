
#pragma once

#include <memory>
#include <filesystem>

namespace google_breakpad
{
    class ExceptionHandler;
    struct CustomInfoEntry;
    struct CustomClientInfo;
}

namespace fs
{
    struct CrashHandler
    {
        CrashHandler( std::filesystem::path && crash_dumps_dir, std::string && additional_version_data );
        ~CrashHandler();

    protected:
        std::unique_ptr< google_breakpad::ExceptionHandler > m_crash_handler;
        std::filesystem::path m_crash_dumps_dir;
        /// @brief {SEMVER} : {GIT_HASH} (Compile time)
        std::string m_additional_version_data;

        std::unique_ptr< google_breakpad::CustomInfoEntry[] > m_custom_info_entries;
        std::unique_ptr< google_breakpad::CustomClientInfo > m_custom_client_info;
    };
}
