cmake_minimum_required(VERSION 2.8)

project(mvbc_lib)

# Include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# mvbc lib tests
add_executable(mvbc_init_test test_init.c)
add_executable(mvbc_read_test test_read.c)
add_executable(mvbc_exit_test test_exit.c)

target_link_libraries(mvbc_init_test PUBLIC mvbc_lib)
target_link_libraries(mvbc_read_test PUBLIC mvbc_lib)
target_link_libraries(mvbc_exit_test PUBLIC mvbc_lib)

# Install target
install(TARGETS mvbc_init_test DESTINATION bin)
install(TARGETS mvbc_read_test DESTINATION bin)
install(TARGETS mvbc_exit_test DESTINATION bin)