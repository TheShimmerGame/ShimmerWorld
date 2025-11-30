
function(shimmer_add_doctest target)
    if (ARGC LESS 2)
        message(FATAL_ERROR "shimmer_add_doctest(${target} ...): At least one source file is required.")
    endif()

    set(sources ${ARGN})

    add_executable(${target} ${sources})
    target_link_libraries(${target}
        PRIVATE
            shimmer::shared
            doctest::doctest)

    doctest_discover_tests(${target} ADD_LABELS 1)
endfunction()

shimmer_add_doctest(shm_config_tests config/ConfigTest.cpp)
shimmer_add_doctest(shm_logging_tests logging/LoggingTest.cpp)
