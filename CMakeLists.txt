cmake_minimum_required(VERSION 3.15)

project(parflowio VERSION 0.0.1 LANGUAGES CXX)
message(STATUS "project: ${PROJECT_NAME}")
message(STATUS "version: ${PROJECT_VERSION}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(PROJECT_VERSION 0.0.1)
# Only do these if this is the main project, and not if it is included through add_subdirectory

set(parflowio_cmake_dir "${parflowio_SOURCE_DIR}/CMake")
list(INSERT CMAKE_MODULE_PATH 0 "${parflowio_cmake_dir}")

# Force default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug, Release (default), RelWithDebInfo and MinSizeRel."
    FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
   # include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

include(parflowioCMakeBackports)
add_subdirectory(src)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    #enable_testing()
    #include(Ctest)
    add_subdirectory(tests)
endif()
# By default only the C++ library is built.
option(BUILD_CXX "Build C++ library" ON)
option(BUILD_PYTHON "Build Python Library" OFF)

message(STATUS "Build C++ library: ${BUILD_CXX}")
message(STATUS "Build Python: ${BUILD_PYTHON}")

if(BUILD_PYTHON)
    add_subdirectory(python)
endif()
