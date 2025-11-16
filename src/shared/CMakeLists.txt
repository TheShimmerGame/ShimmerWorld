
set( TARGET_NAME shared_shim )

add_library( ${TARGET_NAME} STATIC )
add_library( shimmer::shared ALIAS shared_shim )

### LIBRARIES
find_package(flecs CONFIG REQUIRED)
find_package(args CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(unofficial-breakpad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set_target_properties( ${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX )

target_include_directories( ${TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file( GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS *.cpp *.hpp )

target_sources( ${TARGET_NAME} 
    PRIVATE 
        ${TARGET_SOURCES}
)

if (WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC   
       WIN32_LEAN_AND_MEAN
       NOMINMAX
       
       #PUBLIC  # define minimal windows version to 7. Boost::asio will yell at you constantly if you don't set this
       _WIN32_WINNT=0x0601
    )   
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TARGET_SOURCES})
#target_include_directories( ${TARGET_NAME} PUBLIC ${CPP_REFL_INCLUDE_DIRS})
target_link_libraries( ${TARGET_NAME}
    PUBLIC
		shimmer::common
		flecs::flecs_static
		taywee::args
		magic_enum::magic_enum
		spdlog::spdlog # possibly private and wrap in our log system
    PRIVATE
		unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client
)
