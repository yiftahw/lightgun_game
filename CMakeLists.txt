cmake_minimum_required(VERSION 3.21)

project(lightgun_game)

set(PRJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${PRJ_ROOT}/src)
set(INC_DIR ${PRJ_ROOT}/inc)

# 3rd party libraries
option(INSTALL_DEPS "Install dependencies" OFF)
if(INSTALL_DEPS STREQUAL "ON")
    include(${PRJ_ROOT}/cmake/get_cpm.cmake)
    include(${PRJ_ROOT}/cmake/cpr.cmake)
    include(${PRJ_ROOT}/cmake/sdl2.cmake)
    include(${PRJ_ROOT}/cmake/cli11.cmake)
endif()

# setup
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic")

find_package(SDL2 REQUIRED COMPONENTS SDL2 SDL2main)
find_package(cpr REQUIRED)
find_package(CLI11 REQUIRED)

# define the lightgun_game executable
set(APP_SRCS
    ${SRC_DIR}/screen.cpp
    ${SRC_DIR}/Snapshot.cpp
    ${SRC_DIR}/DataAcqHTTP.cpp
    ${SRC_DIR}/DataAcqPlayback.cpp
    ${SRC_DIR}/Geometry.cpp
    ${SRC_DIR}/PointMapping.cpp
    ${SRC_DIR}/main.cpp)

set(APP_INC_DIRS
    ${INC_DIR})

add_executable(lightgun_game ${APP_SRCS})
target_include_directories(lightgun_game PUBLIC ${APP_INC_DIRS})

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(lightgun_game PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(lightgun_game PRIVATE cpr::cpr SDL2::SDL2-static CLI11::CLI11)
