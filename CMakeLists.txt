
## TODO Update the versions and vcpkg
cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(cmake/vcpkg/Include.cmake)

# we need to set it here instead of e.g. the CmakeList file of editor because VS_STARTUP_PROJECT requires to be in the same file scope as project call
set( BINARY_TARGET_NAME "the_shimmer" )

project(shimmerworld)
set(WEDITOR_SEM_VER_MAJOR "0")
set(WEDITOR_SEM_VER_MINOR "0")
set(WEDITOR_SEM_VER_PATCH "1")

include(cmake/vcpkg/Include.cmake)
include(cmake/git/CheckGit.cmake)
CheckGitSetup()

### OPTIONS
# TODO Move me to cmake/options.cmake
cmake_dependent_option(BUILD_TESTING "Build tests" ON [["build-tests" IN_LIST VCPKG_MANIFEST_FEATURES]] ON )

find_package( doctest CONFIG REQUIRED )
include(doctest)
if (BUILD_TESTING)
	include(CTest)
endif()

add_subdirectory(src)

if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
    # This only works in the same file scope as the project call
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${BINARY_TARGET_NAME})
endif()
