cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(league_tracker VERSION 0.1)
add_subdirectory(gil_asset_manager)
add_subdirectory(raylib)
add_subdirectory(gil_riot)

set(main_target tracker)
add_executable(${main_target} main.cpp)
configure_file(config.h.in config.h)
target_link_libraries(${main_target} PUBLIC raylib gilassetmanager gilriot)
target_include_directories(${main_target} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")

file(COPY assets DESTINATION ${PROJECT_BINARY_DIR})

