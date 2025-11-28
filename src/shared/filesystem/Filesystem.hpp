
#pragma once

#include <filesystem>
#include <iosfwd>
#include <vector>

#include "results/Result.hpp"

// TODO: Get rid of filesystem include
namespace shm::fs
{
    shm::Result< std::string > ReadFileToString( const std::filesystem::path & path );
    shm::Result< void > WriteStringToFile( const std::filesystem::path & path, std::string_view data, std::ios_base::openmode mode );
    shm::Result< void > CreateDirectories( const std::filesystem::path & path );

    shm::Result< std::string > GetFileName( const std::filesystem::path & path );
} // namespace shm::fs
