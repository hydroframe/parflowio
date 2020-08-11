# 'test' is the subproject name
project(tests)
include(GoogleTest)
# 'googletest' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(parflowio PUBLIC ../include)

add_executable(run_tests PFData_test.cpp)
add_dependencies(run_tests gtest)
include_directories(${source_dir}/include)
target_link_libraries(run_tests PRIVATE parflowio gtest gtest_main)

add_custom_target(test run_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "make test"
        VERBATIM )

#target_link_libraries(run_tests GTest::GTest)
#gtest_discover_tests(run_tests)
add_test( NAME run_tests COMMAND run_tests)
        #COMMAND run_tests
        #WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


