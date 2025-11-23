
add_subdirectory(common)
add_subdirectory(shared)
add_subdirectory(worldbroker)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
