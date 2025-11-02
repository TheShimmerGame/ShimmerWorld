include(${CMAKE_CURRENT_LIST_DIR}/vcpkg_find_fbxsdk.cmake)

vcpkg_find_fbxsdk(OUT_FBXSDK_ROOT FBXSDK_ROOT)

add_library(FBXSDK INTERFACE IMPORTED)

set_target_properties(FBXSDK PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${FBXSDK_ROOT}/include"
  INTERFACE_LINK_LIBRARIES "${FBXSDK_ROOT}/lib/vs2019/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk-md.lib;${FBXSDK_ROOT}/lib/vs2019/x64/$<IF:$<CONFIG:Debug>,debug,release>/libxml2-md.lib"
)