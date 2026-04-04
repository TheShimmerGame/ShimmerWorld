
set(TargetBinName "Worldbroker")
add_executable(${TargetBinName} main.cpp)

target_link_libraries(${TargetBinName}
	PRIVATE
		shimmer::common
        shimmer::shared)

# TODO: this needs cleaning up, i've just copy n pasted this from the internets and that never ends well
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    target_compile_options(${TargetBinName} PRIVATE
		-fansi-escape-codes -fcolor-diagnostics
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
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${TargetBinName} PRIVATE
        /bigobj
        /Zf
        /MP
        /Zi
        /W3
    )
    target_link_options(${TargetBinName} PRIVATE
        $<$<CONFIG:Release>:/DEBUG;/OPT:REF;/OPT:ICF>
        $<$<NOT:$<CONFIG:Release>>:/DEBUG>
    )
	
	# When in debug mode, the VS debugger working directory is in {project}/bin/Debug
	# otherwise in {project}/bin/Release
	#otherwise an empty string is generated
	set(DEBUGGER_WORKING_DIR_GENEX
		$<IF:$<CONFIG:Debug>,${CMAKE_SOURCE_DIR}/bin/Debug,$<IF:$<CONFIG:Release>,${CMAKE_SOURCE_DIR}/bin/Release,"">>)
	set_property(TARGET ${TargetBinName} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${DEBUGGER_WORKING_DIR_GENEX})
endif()

