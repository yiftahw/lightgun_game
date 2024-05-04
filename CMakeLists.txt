cmake_minimum_required(VERSION 3.21)

project(lightgun_game)

set(PRJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${PRJ_ROOT}/src)
set(INC_DIR ${PRJ_ROOT}/inc)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PRJ_ROOT}/cmake)

# vs code intellisense support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# c++ revision
set(CXX_STANDARD 17)

# use flags error, all, extra, pedantic
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic")

# Create an option to switch between a system sdl library and a vendored sdl library
option(MYGAME_VENDORED "Use vendored libraries" OFF)
if(MYGAME_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

# add the cpr library
find_package(cpr REQUIRED)

# define the lightgun_game executable
set(APP_SRCS
    ${SRC_DIR}/screen.cpp
    ${SRC_DIR}/http_client.cpp
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
target_link_libraries(lightgun_game PRIVATE cpr::cpr SDL2::SDL2-static)
