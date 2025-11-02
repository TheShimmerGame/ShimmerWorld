vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO dalerank/imspinner
    REF master
    SHA512 651f302ee65a18ce5d42f3b9400df68381e00a3213080d62daace60ac89199c6d1fb990710365e2a622f6eb62f01c4239c76800c79edeed8dea2948b4017d30a
    HEAD_REF master
)

# Replace the original CMakeLists.txt with our custom one
file(REMOVE "${SOURCE_PATH}/CMakeLists.txt")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/ProjectConfig.cmake.in" DESTINATION "${SOURCE_PATH}")

set(extra_options "")
if("demo" IN_LIST FEATURES)
    list(APPEND extra_options -DIMSPINNER_DEMO=ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${extra_options}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME imspinner CONFIG_PATH lib/cmake/imspinner)
vcpkg_copy_pdbs()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
