# This package doesn't install FBX SDK. It instead verifies that FBX SDK is installed.

include(${CMAKE_CURRENT_LIST_DIR}/vcpkg_find_fbxsdk.cmake)

vcpkg_find_fbxsdk(OUT_FBXSDK_ROOT FBXSDK_ROOT)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/vcpkg_find_fbxsdk.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/fbxsdk-config.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})

set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
