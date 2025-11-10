
# BINARY_TARGET_NAME defined in root cmakelists file because in case of VS generator, we want to set startup project, which requires filescope to be the same as project( .. ) call
add_executable( ${BINARY_TARGET_NAME} )

set_target_properties( ${BINARY_TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX )

if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
    # When in debug mode, the VS debugger working directory is in {project}/bin/Debug
    # otherwise in {project}/bin/Release
    #otherwise an empty string is generated
    set(DEBUGGER_WORKING_DIR_GENEX
        $<IF:$<CONFIG:Debug>,${CMAKE_SOURCE_DIR}/bin/Debug,$<IF:$<CONFIG:Release>,${CMAKE_SOURCE_DIR}/bin/Release,"">>)
    set_property(TARGET ${BINARY_TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${DEBUGGER_WORKING_DIR_GENEX})
endif()

target_include_directories( ${BINARY_TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features( ${BINARY_TARGET_NAME} PRIVATE cxx_std_23 )

file( GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS *.cpp *.hpp )

target_sources( ${BINARY_TARGET_NAME}
    PRIVATE
        ${TARGET_SOURCES} )

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TARGET_SOURCES})

### LIBRARIES
find_package(flecs CONFIG REQUIRED)
find_package(args CONFIG REQUIRED)

# TODO: this needs cleaning up, i've just copy n pasted this from the internets and that never ends well
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BINARY_TARGET_NAME} PRIVATE
		#-fansi-escape-codes -fcolor-diagnostics
        -Wall -Wextra -Wpedantic
        -Wconversion
        -Wshadow-all
        -Wimplicit-fallthrough
        -Wformat -Wformat=2 -Werror=format-security
		-Wno-cast-function-type-mismatch
		# This warning doesn't work properly in VS, it reports even in proper header
		# https://github.com/mitaki28/vscode-clang/issues/53
		-Wno-pragma-once-outside-header
		-Wno-c++98-compat
        -g)
elseif (MSVC)
    target_compile_options(${BINARY_TARGET_NAME} PRIVATE
        /bigobj
        /Zf
        /MP
        /Zi
        /W3
    )
    target_link_options(${BINARY_TARGET_NAME} PRIVATE
        $<$<CONFIG:Release>:/DEBUG;/OPT:REF;/OPT:ICF>
        $<$<NOT:$<CONFIG:Release>>:/DEBUG>
    )
endif()

target_link_libraries( ${BINARY_TARGET_NAME}
    PUBLIC
		shimmer::common
		shimmer::shared
		
		flecs::flecs_static
		taywee::args
)

