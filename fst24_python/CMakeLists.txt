cmake_minimum_required(VERSION 3.15)
project(fst24_python)

# Options
option(BUILD_TESTING "Build test suite" ON)
option(USE_OPENMP "Enable OpenMP support" ON)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(OpenMP)

# Find RMN library
find_library(RMNCLIB rmn)
if(NOT RMNCLIB)
    message(FATAL_ERROR "RMN library not found")
endif()

# Configure compiler
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(USE_OPENMP AND OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

# Source files
set(SOURCES
    src/fst24_python.c
    src/core_file.c
    src/metadata.c
    src/data_access.c
    src/grid_ops.c
)

# Python module
Python_add_library(fst24_python MODULE ${SOURCES})
target_link_libraries(fst24_python PRIVATE
    ${RMNCLIB}
    ${Python_LIBRARIES}
)
if(USE_OPENMP AND OpenMP_C_FOUND)
    target_link_libraries(fst24_python PRIVATE OpenMP::OpenMP_C)
endif()

# Installation
install(TARGETS fst24_python
    LIBRARY DESTINATION ${Python_SITEARCH}
)

# Testing
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
