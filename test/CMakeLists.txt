add_subdirectory(utilities)

include_directories(utilities/include)

add_subdirectory(compressors_tests)
add_subdirectory(utils_test)
add_subdirectory(compression_test)
add_subdirectory(network)
add_subdirectory(messaging)
add_subdirectory(monitors)