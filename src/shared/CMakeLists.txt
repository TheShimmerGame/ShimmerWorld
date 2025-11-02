
set( TARGET_NAME shared_shim )

add_library( ${TARGET_NAME} STATIC )
add_library( shimmer::shared ALIAS shared_shim )

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

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TARGET_SOURCES})
target_include_directories( ${TARGET_NAME} PUBLIC ${CPP_REFL_INCLUDE_DIRS})
target_link_libraries( ${TARGET_NAME}
    PUBLIC

    PRIVATE
)
