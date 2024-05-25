cmake_minimum_required(VERSION 3.20)
project(dispersion)

set(CMAKE_CXX_STANDARD 20)

find_package(ImGui REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(SFML REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics imgui::imgui imgui-sfml::imgui-sfml "-ObjC")
