
# a cmake file for a test build 
# For test 
include_directories (${CMAKE_SOURCE_DIR}/src)

## have to do this for each test
add_executable(test_pipe src/Elements/test_pipe.cpp)
target_link_libraries(test_pipe LINK_PUBLIC epanet3)
add_test( pipe test_pipe )