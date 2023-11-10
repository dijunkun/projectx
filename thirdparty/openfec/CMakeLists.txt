cmake_minimum_required(VERSION 2.6)

# #project
project(openfec C)

ENABLE_TESTING()

if(PROFILING STREQUAL "ON")
else(PROFILING STREQUAL "ON")
endif(PROFILING STREQUAL "ON")

if(DEBUG STREQUAL "ON")
        # Debug mode
        ADD_DEFINITIONS(-DOF_DEBUG)
        set(CMAKE_BUILD_TYPE Debug)
        message(STATUS "Debug mode ON")

else(DEBUG STREQUAL "ON")
        # Release mode
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_C_FLAGS "-O2")
        message(STATUS "Debug mode OFF")
endif(DEBUG STREQUAL "ON")

# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# include_directories(${PROJECT_SOURCE_DIR}/src/lib_common)
# set(CMAKE_INSTALL_LIBDIR ${PROJECT_SOURCE_DIR}/src/lib_common)
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
MARK_AS_ADVANCED(
        LIBRARY_OUTPUT_PATH
        EXECUTABLE_OUTPUT_PATH
)

link_directories(${LIBRARY_OUTPUT_PATH})

add_subdirectory(src)