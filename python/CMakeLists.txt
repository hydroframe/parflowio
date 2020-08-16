
if(BUILD_PYTHON)
    # Use latest UseSWIG module
    cmake_minimum_required(VERSION 3.14)

    if(NOT TARGET parflowio)
      message(FATAL_ERROR "Python: missing parflowio TARGET")
    endif()

    # Will need swig
    set(CMAKE_SWIG_FLAGS)
    find_package(SWIG REQUIRED)
    include(UseSWIG)

    if(${SWIG_VERSION} VERSION_GREATER_EQUAL 4)
      list(APPEND CMAKE_SWIG_FLAGS "-doxygen")
    endif()

    if(UNIX AND NOT APPLE)
      list(APPEND CMAKE_SWIG_FLAGS "-DSWIGWORDSIZE64")
    endif()

    # Find Python
    find_package(Python REQUIRED COMPONENTS Interpreter Development NumPy)

    if(Python_VERSION VERSION_GREATER_EQUAL 3)
      list(APPEND CMAKE_SWIG_FLAGS "-py3;-DPY3")
    endif()

    # Swig wrap libraries    
    #add_subdirectory(python)
    

    #######################
    ## Python Packaging  ##
    #######################
    # setup.py.in contains cmake variable e.g. @PROJECT_NAME@ and
    # generator expression e.g. $<TARGET_FILE_NAME:pyFoo>
    configure_file(
      setup.py.in
      ${PROJECT_SOURCE_DIR}/python/setup.py.in
      @ONLY)
    file(GENERATE
      OUTPUT parflowio/setup.py
      INPUT ${PROJECT_SOURCE_DIR}/python/setup.py.in)

    # Find if python module MODULE_NAME is available,
    # if not install it to the Python user install directory.
    function(search_python_module MODULE_NAME)
      execute_process(
        COMMAND ${Python_EXECUTABLE} -c "import ${MODULE_NAME}; print(${MODULE_NAME}.__version__)"
        RESULT_VARIABLE _RESULT
        OUTPUT_VARIABLE MODULE_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      if(${_RESULT} STREQUAL "0")
        message(STATUS "Found python module: ${MODULE_NAME} (found version \"${MODULE_VERSION}\")")
      else()
        message(WARNING "Can't find python module \"${MODULE_NAME}\", user install it using pip...")
        execute_process(
          COMMAND ${Python_EXECUTABLE} -m pip install --upgrade --user ${MODULE_NAME}
          OUTPUT_STRIP_TRAILING_WHITESPACE
          )
      endif()
    endfunction()

    # Look for required python modules
    search_python_module(setuptools)
    search_python_module(wheel)

    add_custom_target(python_package ALL
      #COMMAND ${CMAKE_COMMAND} -E copy $<CONFIG>/setup.py setup.py
      COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/python/__init__.py.in ${PROJECT_NAME}/__init__.py
      #COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/python/__init__.py.in ${PROJECT_NAME}/parflowio/__init__.py

      COMMAND ${CMAKE_COMMAND} -E remove_directory dist
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}/.libs
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pyParflowio> ${PROJECT_NAME}
      # Don't need to copy static lib on Windows
      COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:${UNIX}>,copy,true>
      $<TARGET_FILE:parflowio_shared> ${PROJECT_NAME}/.libs
      COMMAND ${Python_EXECUTABLE} setup.py bdist_wheel
      BYPRODUCTS
      python/${PROJECT_NAME}
      python/build
      python/dist
      python/${PROJECT_NAME}.egg-info
      WORKING_DIRECTORY parflowio
      )

    # Test
    if(BUILD_TESTING)
      # Look for python module virtualenv
      search_python_module(virtualenv)
      # Testing using a vitual environment
      set(VENV_EXECUTABLE ${Python_EXECUTABLE} -m virtualenv)
      set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/venv)
      if(WIN32)
        set(VENV_Python_EXECUTABLE "${VENV_DIR}\\Scripts\\python.exe")
      else()
        set(VENV_Python_EXECUTABLE ${VENV_DIR}/bin/python)
      endif()
      # make a virtualenv to install our python package in it
      add_custom_command(TARGET python_package POST_BUILD
        COMMAND ${VENV_EXECUTABLE} -p ${Python_EXECUTABLE} ${VENV_DIR}
        # Must not call it in a folder containing the setup.py otherwise pip call it
        # (i.e. "python setup.py bdist") while we want to consume the wheel package
        COMMAND ${VENV_Python_EXECUTABLE} -m pip install --find-links=${CMAKE_CURRENT_BINARY_DIR}/python/dist ${PROJECT_NAME}
        BYPRODUCTS ${VENV_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
      # run the tests within the virtualenv
      add_test(NAME pytest_venv
        COMMAND ${VENV_Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/python/test.py)
    endif()
endif()

set_property(SOURCE parflowio.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE parflowio.i PROPERTY SWIG_MODULE_NAME pyParflowio)
swig_add_library(pyParflowio
  TYPE SHARED
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/python/${PROJECT_NAME}/parflowio
  SOURCES parflowio.i)

target_include_directories(pyParflowio
  PRIVATE
  ../include
  ${Python_INCLUDE_DIRS}
  )
set_property(TARGET pyParflowio PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)

# note: macOS is APPLE and also UNIX !
if(APPLE)
  set_target_properties(pyParflowio PROPERTIES
    SUFFIX ".so"
    INSTALL_RPATH "@loader_path;@loader_path/../../${PROJECT_NAME}/.libs"
    )
  set_property(TARGET pyParflowio APPEND PROPERTY
    LINK_FLAGS "-flat_namespace -undefined suppress"
    )
elseif(UNIX)
  set_target_properties(pyParflowio PROPERTIES
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../../${PROJECT_NAME}/.libs"
    )
endif()
target_link_libraries(pyParflowio PRIVATE parflowio_shared)

# Variable PYTHON_LIBRARIES can contains keyword `optimized`
# which won't be interpreted inside a generator expression.
# i.e. we can't use: $<$<PLATFORM_ID:Windows>:${PYTHON_LIBRARIES}>
# see: https://cmake.org/cmake/help/git-stage/command/target_link_libraries.html#command:target_link_libraries
if(MSVC)
  target_link_libraries(pyParflowio PRIVATE ${Python_LIBRARIES})
endif()
