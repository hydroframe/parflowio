set_property(SOURCE parflowio.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE parflowio.i PROPERTY SWIG_MODULE_NAME pyParflowio)
swig_add_library(pyParflowio
  TYPE SHARED
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/python/${PROJECT_NAME}/Parflowio
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
target_link_libraries(pyParflowio PRIVATE Parflowio)

# Variable PYTHON_LIBRARIES can contains keyword `optimized`
# which won't be interpreted inside a generator expression.
# i.e. we can't use: $<$<PLATFORM_ID:Windows>:${PYTHON_LIBRARIES}>
# see: https://cmake.org/cmake/help/git-stage/command/target_link_libraries.html#command:target_link_libraries
if(MSVC)
  target_link_libraries(pyParflowio PRIVATE ${Python_LIBRARIES})
endif()
