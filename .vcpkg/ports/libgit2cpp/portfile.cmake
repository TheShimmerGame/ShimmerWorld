vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO AndreyG/libgit2cpp
    REF e9651575e388d7e5832ff64955b2f3304bac33db
    SHA512 ff5320a37b89bab937766fff58dc78d0bbec783f7971e8545657de3b01e81449a02eef301608469504d1e8ecf9d24f08e2bb427dab75c4105e7a6591de22ef69
    HEAD_REF master
)

# Replace the original CMakeLists.txt with our custom one
file(REMOVE "${SOURCE_PATH}/CMakeLists.txt")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

set(USE_BOOST OFF)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUNDLE_LIBGIT2=OFF
        -DUSE_BOOST=${USE_BOOST}
        -DBUILD_LIBGIT2CPP_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME git2cpp CONFIG_PATH share/git2cpp)
vcpkg_copy_pdbs()

# Handle copyright
if(EXISTS "${SOURCE_PATH}/LICENSE")
    vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
else()
    # If no LICENSE file exists, use current date in copyright note
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "Copyright (c) 2025 AndreyG\nLibgit2cpp is released under MIT license.")
endif()

# Remove unnecessary files
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")