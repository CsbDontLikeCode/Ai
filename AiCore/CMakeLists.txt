cmake_minimum_required(VERSION 3.10)
project(AiCore)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd" CACHE STRING "Flags used by the compiler during debug builds.")

file(GLOB_RECURSE CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*)
file(GLOB_RECURSE CORE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h*)

add_library(${PROJECT_NAME} STATIC ${CORE_SOURCES} ${CORE_INCLUDES})

target_sources(${PROJECT_NAME} PRIVATE ../dependencies/stb/stb_image.h)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "../dependencies/glad/include"
        "../dependencies/glfw/include"
        "../dependencies/assimp/include"
        "${CMAKE_BINARY_DIR}/dependencies/assimp/include"
        "../dependencies/glm/"
        "../dependencies/stb"
        "../dependencies/imgui"
)

target_link_libraries(${PROJECT_NAME} PRIVATE INTERFACE
        glm
        glfw
        glad
        assimp::assimp
        imgui
        opengl32.lib)

target_compile_options(${PROJECT_NAME} PRIVATE /W3)