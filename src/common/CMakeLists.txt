
set( TARGET_NAME common_shim )

add_library( ${TARGET_NAME} STATIC )
add_library( shimmer::common ALIAS common_shim )

file( GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS *.cpp *.hpp )
target_sources( ${TARGET_NAME} 
    PRIVATE 
        ${TARGET_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TARGET_SOURCES})

target_include_directories( ${TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

#target_link_libraries( ${TARGET_NAME}
#	PRIVATE
#)

