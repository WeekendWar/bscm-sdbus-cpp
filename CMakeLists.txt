cmake_minimum_required(VERSION 3.15)
project(bscm-sdbus-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUS_CPP REQUIRED IMPORTED_TARGET sdbus-c++)

# Source files
set(SOURCES
    src/main.cpp
    src/BluetoothManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${SDBUS_CPP_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
  PkgConfig::SDBUS_CPP
  # ${SDBUS_CPP_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# ##############################################################################
# # Install targets                                                           ##
# ##############################################################################

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
        RUNTIME DESTINATION bin
)

# Export pkg-config file if desired (optional, not included here)