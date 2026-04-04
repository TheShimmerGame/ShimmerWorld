
macro(shimmer_glob_module_sources OUTPUT_VAR)
    if(ARGN)
        file(GLOB_RECURSE ${OUTPUT_VAR} ${ARGN})
    else()
        file(GLOB_RECURSE ${OUTPUT_VAR} *.ixx */*.ixx *.cppm */*.cppm)
    endif()
endmacro()
