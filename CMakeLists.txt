cmake_minimum_required(VERSION 3.10)
project(bscm-sdbus-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

# Source files
set(SOURCES
    src/main.cpp
    src/BluetoothManager.cpp
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDBUS_CPP_INCLUDE_DIRS}
)

# Create executable
add_executable(bscm ${SOURCES})

# Link libraries
target_link_libraries(bscm
    ${SDBUS_CPP_LIBRARIES}
)

# Compiler flags
target_compile_options(bscm PRIVATE -Wall -Wextra)
