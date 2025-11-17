
#include "Filesystem.hpp"

#include <fstream>

shm::Result< void > shm::fs::WriteStringToFile( const std::filesystem::path & path, std::string_view data, std::ios_base::openmode mode )
{
    std::fstream file( path, mode );
    if ( !file.is_open() )
        return std::unexpected( std::make_error_code( static_cast< std::errc >( errno ) ) );

    file << data;
    file.close();
    return {};
}

shm::Result< void > shm::fs::CreateDirectories( const std::filesystem::path & path )
{
    std::error_code ec{};
    std::filesystem::create_directories( path, ec );
    if ( ec )
        return std::unexpected( ec );

    return {};
}

shm::Result< std::string > shm::fs::ReadFileToString( const std::filesystem::path & path )
{
    std::vector< std::byte > buffer{};

    std::ifstream file( path );
    if ( !file.is_open() )
        return std::unexpected( std::make_error_code( static_cast< std::errc >( errno ) ) );

    std::string content( std::istreambuf_iterator< char >{ file }, {} );
    return content;
}
