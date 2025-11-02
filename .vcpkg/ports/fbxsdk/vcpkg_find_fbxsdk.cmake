function(vcpkg_find_fbxsdk)
    cmake_parse_arguments(PARSE_ARGV 0 vfc "" "OUT_FBXSDK_ROOT" "")

    if(NOT vfc_OUT_FBXSDK_ROOT)
        message(FATAL_ERROR "vcpkg_find_fbxsdk() requres an OUT_FBXSDK_ROOT argument")
    endif()

    find_file(FBX_HEADER
            NAMES fbxsdk.h
            DOC "Toolkit location."
            PATHS ENV FBXSDK_HEADER_LOCATION
        )
    
    if(${FBX_HEADER} STREQUAL "FBX_HEADER-NOTFOUND")
        message(FATAL_ERROR "Could not find FBX SDK. Before continuing, please download and install FBX SDK from:"
                            "\n    https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1"
                            "\n and set enviroment variable FBXSDK_HEADER_LOCATION to directory containing fbxsdk.h")
    endif()

    get_filename_component(FBXSDK_ROOT "${FBX_HEADER}" DIRECTORY)
    get_filename_component(FBXSDK_ROOT "${FBXSDK_ROOT}" DIRECTORY)
    set(${vfc_OUT_FBXSDK_ROOT} "${FBXSDK_ROOT}" PARENT_SCOPE)
endfunction()
