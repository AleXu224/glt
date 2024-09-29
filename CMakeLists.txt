cmake_minimum_required(VERSION 3.21)
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 23)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache in ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
    message(STATUS "No ccache")
endif()

project(glt VERSION 0.1.0)

# Shader compilation:
add_executable(ShaderHeaderGenerator shaderHeaderGenerator.cpp)
if (NOT DEFINED GLT_SHADERS)
    file(GLOB_RECURSE shader_files "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.vert" "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.frag")
else()
    file(GLOB_RECURSE shader_files "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.vert" "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.frag" ${GLT_SHADERS})
endif()
add_custom_target(Shaders)
# Create the compiled shaders folder
add_custom_command(TARGET Shaders PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/compiledShaders")
# Compile the shaders into shaders/compiled
foreach(shader_file ${shader_files})
    get_filename_component(dir ${shader_file} DIRECTORY)
    get_filename_component(fileName ${shader_file} NAME)
    add_custom_command(TARGET Shaders PRE_BUILD
    COMMAND glslc ${shader_file} -o "${PROJECT_BINARY_DIR}/compiledShaders/${fileName}.spv")
endforeach()
# Create the compiledShaders directory
add_custom_command(TARGET Shaders PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/include/engine/compiledShaders")
# Run ShaderHeaderGenerator on all shaders to generate a header with the bytecode in include/compiledShaders
foreach(shader_file ${shader_files})
    get_filename_component(fileName ${shader_file} NAME)
    get_filename_component(fileDir ${shader_file} DIRECTORY)
    add_custom_command(TARGET Shaders PRE_BUILD
    COMMAND ShaderHeaderGenerator "${PROJECT_BINARY_DIR}/compiledShaders" ${fileDir} ${fileName} "${PROJECT_SOURCE_DIR}/include/engine/compiledShaders")
endforeach()

# add_library(gltPCH INTERFACE)
# target_precompile_headers(gltPCH INTERFACE 
#     [["window.hpp"]]
#     [["vec2.hpp"]]
#     [["rect.hpp"]]
#     [["box.hpp"]]
#     [["column.hpp"]]
#     [["row.hpp"]]
#     [["stack.hpp"]]
#     [["gestureDetector.hpp"]]
#     [["align.hpp"]]
# )


file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h" "include/*.cpp" "external/*.h")
add_library(glt STATIC ${all_files})
target_compile_features(glt PUBLIC cxx_std_23)
set_target_properties(glt PROPERTIES CXX_EXTENSIONS OFF)
add_dependencies(glt Shaders)
target_compile_definitions(glt PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN ASIO_STANDALONE GLFW_INCLUDE_VULKAN)
target_compile_definitions(glt PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=0)
IF (WIN32)
    target_compile_definitions(glt PRIVATE GLFW_EXPOSE_NATIVE_WIN32 VK_USE_PLATFORM_WIN32_KHR)
ENDIF()
IF (UNIX)
    target_compile_definitions(glt PRIVATE GLFW_EXPOSE_NATIVE_X11 VK_USE_PLATFORM_XLIB_KHR)
ENDIF()
# target_link_libraries(glt PUBLIC gltPCH)
target_include_directories(glt PUBLIC include)
target_include_directories(glt PUBLIC include/engine)
target_include_directories(glt PUBLIC external/fonts)

if (MSVC)
    # add_compile_options(/W4 /WX)
    # add_compile_options(/fsanitize=address)
    # target_compile_options(glt PUBLIC /fsanitize=address)
    target_compile_definitions(glt PUBLIC _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
else()
    target_compile_options(glt PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-c++11-narrowing -Wno-narrowing -Werror)
    # target_link_options(glt PUBLIC -fsanitize=address,undefined)
    # target_link_options(glt PUBLIC -fsanitize=undefined,thread)
endif()

add_executable(gltDemo EXCLUDE_FROM_ALL main.cpp)

enable_testing()

find_package(freetype CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(skyr-url CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SailC++ CONFIG REQUIRED)

target_link_libraries(glt PRIVATE utf8::cpp)
target_link_libraries(glt PRIVATE skyr::skyr-url)
target_link_libraries(glt PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(glt PUBLIC asio::asio)
target_link_libraries(glt PUBLIC freetype)
target_link_libraries(glt PUBLIC glfw)
target_link_libraries(glt PUBLIC Vulkan::Headers)
target_link_libraries(glt PUBLIC glm::glm)
target_link_libraries(glt PRIVATE SAIL::sail-c++)
if (WIN32)
    target_link_libraries(glt PRIVATE "Dwmapi.lib")
    target_link_libraries(glt PRIVATE "version.lib")
endif()

file(GLOB_RECURSE all_test_files CONFIGURE_DEPENDS "tests/*.cc")
add_executable(gltTest EXCLUDE_FROM_ALL ${all_test_files})
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(gltTest PRIVATE Catch2::Catch2WithMain)
target_link_libraries(gltTest PRIVATE glt)

include(CTest)
include(Catch)
catch_discover_tests(gltTest)

target_link_libraries(gltDemo PRIVATE glt)