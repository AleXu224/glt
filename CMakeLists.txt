cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 23)
project(glt VERSION 0.1.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if (MSVC)
    # add_compile_options(/W4 /WX)
    add_compile_options(/fsanitize=address)
    # MSVC is stupid. For some god forsaken reason you need to following to even compile
    add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror -ferror-limit=0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Shader compilation:
add_executable(ShaderHeaderGenerator shaderHeaderGenerator.cpp)
file(GLOB_RECURSE shader_files "${CMAKE_SOURCE_DIR}/src/engine/shaders/*.vert" "${CMAKE_SOURCE_DIR}/src/engine/shaders/*.frag")
add_custom_target(Shaders)
# Create the compiled shaders folder
add_custom_command(TARGET Shaders PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/compiledShaders")
# Compile the shaders into shaders/compiled
foreach(shader_file ${shader_files})
    get_filename_component(dir ${shader_file} DIRECTORY)
    get_filename_component(fileName ${shader_file} NAME)
    add_custom_command(TARGET Shaders PRE_BUILD
    COMMAND glslc ${shader_file} -o "${CMAKE_BINARY_DIR}/compiledShaders/${fileName}.spv")
endforeach()
# Create the compiledShaders directory
add_custom_command(TARGET Shaders PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/include/engine/compiledShaders")
# Run ShaderHeaderGenerator on all shaders to generate a header with the bytecode in include/compiledShaders
foreach(shader_file ${shader_files})
    get_filename_component(fileName ${shader_file} NAME)
    add_custom_command(TARGET Shaders PRE_BUILD
    COMMAND ShaderHeaderGenerator "${CMAKE_BINARY_DIR}/compiledShaders" "${CMAKE_SOURCE_DIR}/src/engine/shaders" ${fileName} "${CMAKE_SOURCE_DIR}/include/engine/compiledShaders")
endforeach()

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h" "external/*.h")
add_library(glt STATIC ${all_files})
add_dependencies(glt Shaders)
target_compile_definitions(glt PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN ASIO_STANDALONE STB_IMAGE_IMPLEMENTATION GLFW_EXPOSE_NATIVE_WIN32 GLFW_INCLUDE_VULKAN VK_USE_PLATFORM_WIN32_KHR)
target_include_directories(glt PUBLIC include)
target_include_directories(glt PRIVATE include/engine)

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
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(glt PRIVATE utf8::cpp)
target_link_libraries(glt PRIVATE skyr::skyr-url)
target_link_libraries(glt PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(glt PUBLIC asio::asio)
target_link_libraries(glt PUBLIC freetype)
target_link_libraries(glt PUBLIC glfw)
target_link_libraries(glt PUBLIC Vulkan::Vulkan)
target_link_libraries(glt PUBLIC glm::glm)
target_link_libraries(glt PUBLIC glfw)
target_link_libraries(glt PRIVATE "Dwmapi.lib")
target_link_libraries(glt PRIVATE "version.lib")

target_link_libraries(gltTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(gltTest PRIVATE glt)
include(GoogleTest)

gtest_discover_tests(gltTest)

target_link_libraries(gltDemo PRIVATE glt)