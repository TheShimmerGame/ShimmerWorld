
add_executable( shm_config_tests config/ConfigTest.cpp )
target_link_libraries( shm_config_tests
	PRIVATE
		shimmer::shared
	    doctest::doctest )
		
# Discover individual doctest TEST_CASEs and register them with CTest

#add_test( NAME shm_config_tests COMMAND $<TARGET_FILE:shm_config_tests> )
doctest_discover_tests( shm_config_tests )
