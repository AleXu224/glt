cmake_minimum_required(VERSION 3.21)
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 23)

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

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h" "external/*.h")
add_library(glt STATIC ${all_files})
add_dependencies(glt Shaders)
target_compile_definitions(glt PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN ASIO_STANDALONE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION GLFW_INCLUDE_VULKAN)
target_compile_definitions(glt PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=0)
IF (WIN32)
    target_compile_definitions(glt PRIVATE GLFW_EXPOSE_NATIVE_WIN32 VK_USE_PLATFORM_WIN32_KHR)
ENDIF()
IF (UNIX)
    target_compile_definitions(glt PRIVATE GLFW_EXPOSE_NATIVE_X11 VK_USE_PLATFORM_XLIB_KHR)
ENDIF()
target_include_directories(glt PUBLIC include)
target_include_directories(glt PUBLIC include/engine)
target_include_directories(glt PUBLIC external/fonts)

if (MSVC)
    # add_compile_options(/W4 /WX)
    # add_compile_options(/fsanitize=address)
    target_compile_definitions(glt PRIVATE _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
else()
    target_compile_options(glt PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-c++11-narrowing -Werror -ferror-limit=0)
endif()

add_executable(gltDemo main.cpp)

enable_testing()

find_package(freetype CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(skyr-url CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
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

# file(GLOB_RECURSE all_test_files CONFIGURE_DEPENDS "tests/*.cc")
# add_executable(gltTest ${all_test_files})
# target_link_libraries(gltTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
# target_link_libraries(gltTest PRIVATE glt)
# include(GoogleTest)

# gtest_discover_tests(gltTest)

target_link_libraries(gltDemo PRIVATE glt)