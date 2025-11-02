vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO altschuler/imgui-knobs
    REF 4f207526f9ef036a0aff7edfaad92cfbe12d987a
    SHA512 8f2b386cca23b4573af1d5d53c68df50841dd8cb99d52968cbd62462ec40e9f7da1000a9a8af28924717d43eac374141d17ad33ff32dbfa4f556e73dc01877cb
    HEAD_REF master
    #Imgui-knobs can be only built statically at the moment
    #PATCHES
    #    fix_lib_export.patch
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
vcpkg_copy_pdbs()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Install license file
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
