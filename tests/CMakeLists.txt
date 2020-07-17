# 'test' is the subproject name
project(tests)
# 'googletest' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(parflowio PUBLIC ../include)

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(run_tests Test1.cpp)
target_link_libraries(run_tests PRIVATE parflowio gtest gtest_main)
