
#include "CrashHandler.hpp"
#include "strings/String.hpp"

#include <client/windows/handler/exception_handler.h>
#include <client/windows/common/ipc_protocol.h>

#include <spdlog/spdlog.h>

constexpr const char * G_CRASH_UPLOADER_BINARY_NAME{ "WoWEditorCrashio.exe" };
constexpr const wchar_t * G_PIPE_NAME = nullptr; // TODO: eventually move this from IP to OOP

static bool DumpCallback( const wchar_t * dump_path, const wchar_t * minidump_id, void * context,
    EXCEPTION_POINTERS * exinfo, MDRawAssertionInfo * assertion, bool succeeded )
{
    // Open process `WoWEditorCrashio.exe` which lies in the same directory as the main executable

    std::filesystem::path dump_path_fs = dump_path;
    std::wstring minidump_id_ws{ minidump_id };
    std::string arguments = fmt::format( R"({} {}\{}.dmp)", G_CRASH_UPLOADER_BINARY_NAME, dump_path_fs.string(), ed::str::ConvertWide( minidump_id ) );

    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    ZeroMemory( &pi, sizeof( pi ) );

    CreateProcess( G_CRASH_UPLOADER_BINARY_NAME, ( LPSTR )arguments.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    return succeeded;
}

namespace fs
{
    CrashHandler::~CrashHandler() = default;
    CrashHandler::CrashHandler( std::filesystem::path && crash_dumps_dir, std::string && add_data )
        : m_crash_handler( nullptr )
        , m_crash_dumps_dir( std::move( crash_dumps_dir ) )
        , m_additional_version_data( std::move( add_data ) )
        , m_custom_info_entries( nullptr )
        , m_custom_client_info( nullptr )
    {
        m_custom_info_entries = std::make_unique< google_breakpad::CustomInfoEntry[] >( 1 );
        std::wstring wadd_data = ed::str::ConvertToWide( m_additional_version_data.data() );
        // Custom info entry supports name-key up to 64 characters, keep this in mind, wadd_data is constructed in editor/main
        // and contains git_hash and semver in format GIT_HASH:SEM_VER
        m_custom_info_entries[ 0 ] = google_breakpad::CustomInfoEntry( L"AdditionalVersionData", wadd_data.c_str() );
        m_custom_client_info = std::make_unique< google_breakpad::CustomClientInfo >();
        m_custom_client_info->entries = m_custom_info_entries.get();

        if ( !std::filesystem::exists( m_crash_dumps_dir ) )
        {
            std::error_code ec;
            if ( !std::filesystem::create_directories( m_crash_dumps_dir, ec ) || ec )
            {
                spdlog::debug( "Failed to create crash dumps directory: {}", ec.message() );
                std::abort();
                return;
            }
        }

        m_crash_handler = std::make_unique< google_breakpad::ExceptionHandler >(
            m_crash_dumps_dir
            , nullptr
            , DumpCallback
            , nullptr
            , google_breakpad::ExceptionHandler::HANDLER_ALL
            , MINIDUMP_TYPE( MINIDUMP_TYPE::MiniDumpNormal | MINIDUMP_TYPE::MiniDumpWithDataSegs | MINIDUMP_TYPE::MiniDumpWithThreadInfo )
            , G_PIPE_NAME
            , m_custom_client_info.get() );
    }

}
