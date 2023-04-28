cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(glt VERSION 0.1.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")

add_executable(glt main.cpp ${all_files})

target_include_directories(glt PUBLIC include)

find_package(glfw3 CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

target_link_libraries(glt PRIVATE freetype)
target_link_libraries(glt PRIVATE glfw)
target_link_libraries(glt PRIVATE "Dwmapi.lib")
target_link_libraries(glt PRIVATE "d3d11.lib")
target_link_libraries(glt PRIVATE "d3dcompiler.lib")
target_link_libraries(glt PRIVATE "version.lib")
