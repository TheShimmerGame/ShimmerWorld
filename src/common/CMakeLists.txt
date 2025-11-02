
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

find_package(unofficial-breakpad CONFIG REQUIRED)
#find_package(Boost REQUIRED COMPONENTS nowide)
find_package(spdlog REQUIRED)

if (WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC   
       WIN32_LEAN_AND_MEAN
       NOMINMAX
       
       #PUBLIC  # define minimal windows version to 7. Boost::asio will yell at you constantly if you don't set this
       _WIN32_WINNT=0x0601
    )   
endif()

target_link_libraries( ${TARGET_NAME}
	PUBLIC
		spdlog::spdlog
	PRIVATE
		# possibly private?
		unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client
		#Boost::nowide
)

