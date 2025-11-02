cmake_minimum_required(VERSION 3.6)

project(libgit2cpp LANGUAGES C CXX)
set(package git2cpp)
include(GNUInstallDirs)

# Set version
set(version 1.0.0)

option(USE_BOOST "Enable use of Boost header libraries" OFF)
option(BUNDLE_LIBGIT2 "Use bundled libgit2" OFF)
option(BUILD_LIBGIT2CPP_EXAMPLES "Build libgit2cpp examples" OFF)

if (USE_BOOST)
    find_package(Boost REQUIRED)
    add_definitions(-DUSE_BOOST=1)
    include_directories(${BOOST_INCLUDEDIR})
endif ()

if (BUNDLE_LIBGIT2)
    add_subdirectory(libs/libgit2)
endif ()

file(GLOB_RECURSE LIBGIT2CPP_SOURCES
    src/*.cpp
    include/git2cpp/*.h
)

add_library(${package} STATIC ${LIBGIT2CPP_SOURCES})
add_library(${package}::${package} ALIAS ${package})

set_target_properties(${package} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    INTERFACE_COMPILE_FEATURES cxx_std_17
    VERSION ${version}
    SOVERSION 1
)

if (MSVC AND (${MSVC_VERSION} VERSION_GREATER 1900))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif ()

target_include_directories(${package}
    PRIVATE
        src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (USE_BOOST)
    target_include_directories(${package} PUBLIC ${Boost_INCLUDE_DIRS})
endif ()

if (BUILD_LIBGIT2CPP_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (BUNDLE_LIBGIT2)
    target_include_directories(${package} PUBLIC libs/libgit2/include)
    target_link_libraries(${package} libgit2package)
else ()
    find_package(libgit2 CONFIG REQUIRED)
    target_link_libraries(${package} libgit2::libgit2package)
endif ()

# Installation rules
install(
    TARGETS ${package}
    EXPORT ${package}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create and install the export targets
install(
    EXPORT ${package}Targets
    FILE ${package}Targets.cmake
    NAMESPACE ${package}::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${package}
)

# Create and install package configuration files
include(CMakePackageConfigHelpers)

# Create Config.cmake.in in the current binary directory
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Config.cmake.in" "
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
find_dependency(libgit2 CONFIG)

include(\"\${CMAKE_CURRENT_LIST_DIR}/${package}Targets.cmake\")

check_required_components(${package})
")

# Configure the package config file
configure_package_config_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${package}
)

# Create version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
    VERSION ${version}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${package}
)