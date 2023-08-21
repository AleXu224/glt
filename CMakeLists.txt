cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 23)
project(glt VERSION 0.1.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if (MSVC)
    add_compile_options(/W4 /WX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ferror-limit=0")
endif()


file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h" "external/*.h")
add_library(glt STATIC ${all_files})
target_include_directories(glt PUBLIC include)

file(GLOB_RECURSE all_test_files CONFIGURE_DEPENDS "tests/*.cc")
add_executable(gltTest ${all_test_files})

add_executable(gltDemo main.cpp)

enable_testing()

find_package(glfw3 CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(skyr-url CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

target_link_libraries(glt PRIVATE utf8::cpp)
target_link_libraries(glt PUBLIC skyr::skyr-url)
target_link_libraries(glt PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(glt PUBLIC asio::asio)
target_link_libraries(glt PUBLIC freetype)
target_link_libraries(glt PUBLIC glfw)
target_link_libraries(glt PRIVATE "Dwmapi.lib")
target_link_libraries(glt PRIVATE "d3d11.lib")
target_link_libraries(glt PRIVATE "d3dcompiler.lib")
target_link_libraries(glt PRIVATE "version.lib")

target_link_libraries(gltTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(gltTest PRIVATE glt)
include(GoogleTest)

gtest_discover_tests(gltTest)

target_link_libraries(gltDemo PRIVATE glt)