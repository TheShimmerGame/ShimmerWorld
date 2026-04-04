
set(WEDITOR_SEM_VER_MAJOR "0")
set(WEDITOR_SEM_VER_MINOR "0")
set(WEDITOR_SEM_VER_PATCH "1")

## TODO Update the versions and vcpkg
cmake_minimum_required(VERSION 4.0 FATAL_ERROR)
include(cmake/vcpkg/Include.cmake)

# Include utilities so each child cmake file can use them
include(cmake/utility/ModuleFileGlob.cmake)

#set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
#set(CMAKE_CXX_MODULE_STD ON)

set(BINARY_TARGET_NAME "the_shimmer")
project(shimmerworld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD_LATEST})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include(cmake/git/CheckGit.cmake)
CheckGitSetup()

### OPTIONS
# TODO Move me to cmake/options.cmake
cmake_dependent_option(BUILD_TESTING "Build tests" ON [["build-tests" IN_LIST VCPKG_MANIFEST_FEATURES]] ON )

find_package(doctest CONFIG REQUIRED)
include(doctest)
if (BUILD_TESTING)
	include(CTest)
endif()

add_subdirectory(src)
