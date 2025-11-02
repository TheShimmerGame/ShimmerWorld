set(AUTO_VCPKG_SPECIFIED_HASH "e3ed41868d5034bc608eaaa58383cd6ecdbb5ffb")
set(AUTO_VCPKG_GIT_TAG "2025.10.17")
string(TIMESTAMP AUTOVCPKG_REMINDER_TIMESTAMP "%Y.%m.%d")
# set this to 3 months after last tag update, new vcpkg tags are released every ~2 months
if (AUTOVCPKG_REMINDER_TIMESTAMP STRGREATER "2025.03.13")
    message(DEPRECATION "AUTO_VCPKG_GIT_TAG wasn't updated for a while. "
    "It is currently set to ${AUTO_VCPKG_GIT_TAG} and today is ${AUTOVCPKG_REMINDER_TIMESTAMP}, it may be good idea to check for new available tags.")
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/AutoVcpkg.cmake)

set( _VCPKG_ROOT )

if ( "${VCPKG_ROOT}" STREQUAL "" )
    set( _VCPKG_ROOT "$ENV{VCPKG_ROOT}" )
else()
    set( _VCPKG_ROOT "${VCPKG_ROOT}" )
endif()

if (EXISTS ${_VCPKG_ROOT})
    set( AUTO_VCPKG_ROOT ${_VCPKG_ROOT} CACHE STRING "auto vcpkg root directory")
else()
    vcpkg_download() 
    set( _VCPKG_ROOT ${AUTO_VCPKG_ROOT} )
endif()

set( VCPKG_ROOT "${_VCPKG_ROOT}" CACHE STRING "vcpkg root directory")
message( STATUS "* VCPKG_ROOT " ${VCPKG_ROOT} )

include( ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake )