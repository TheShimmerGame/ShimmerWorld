vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO btzy/nativefiledialog-extended
    REF "v1.1.1"
    SHA512 2cdcf5563cadc53d22ac1e5741dd685b9763c3cd3bda70f36588db915c3272c4ee9ddfa27c09615b1b86c1aaf0711dfbf86b21eb4332eece43db80c33c38e090
    HEAD_REF master
    PATCHES
        00_config_locations.patch
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" NFD_BUILD_SHARED)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_SHARED_LIBS=${NFD_BUILD_SHARED}
        -DNFD_BUILD_TESTS=OFF
        -DNFD_INSTALL=ON
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
vcpkg_copy_pdbs()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Install license file
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")