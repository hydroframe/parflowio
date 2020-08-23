set(HEADER_LIST "${parflowio_SOURCE_DIR}/include/parflow/pfdata.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(parflowio OBJECT pfdata.cpp ${HEADER_LIST})

# shared libraries need PIC
set_property(TARGET parflowio PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(parflowio_shared SHARED $<TARGET_OBJECTS:parflowio>)
add_library(parflowio_static STATIC $<TARGET_OBJECTS:parflowio>)

set_target_properties(parflowio PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(parflowio PROPERTIES SOVERSION 1)

# We need this directory, and users of our library will need it too
target_include_directories(parflowio PUBLIC ../include)

# All users of this library will need at least C++11
target_compile_features(parflowio PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
