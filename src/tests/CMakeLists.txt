
add_executable( shm_config_tests config/ConfigTest.cpp )
target_link_libraries( shm_config_tests
	PRIVATE
		shimmer::shared
	    doctest::doctest )
		
# Discover individual doctest TEST_CASEs and register them with CTest
doctest_discover_tests( shm_config_tests ADD_LABELS 1 )
