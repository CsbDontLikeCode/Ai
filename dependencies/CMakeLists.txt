cmake_minimum_required(VERSION 3.10)
project(dependencies LANGUAGES CXX)

# glfw
set(GLFW_BUILD_EXAMPLES FALSE CACHE INTERNAL "")
set(GLFW_BUILD_TESTS FALSE CACHE INTERNAL "")
set(GLFW_BUILD_DOCS FALSE CACHE INTERNAL "")
set(GLFW_INSTALL FALSE CACHE INTERNAL "")

#glm
set(GLM_TEST_ENABLE  OFF CACHE INTERNAL "")
set(GLM_ENABLE_CXX_20 TRUE CACHE INTERNAL "")
set(GLM_DISABLE_AUTO_DETECTION TRUE CACHE INTERNAL "")
set(GLM_QUIET TRUE CACHE INTERNAL "")

# 添加子项目
add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(glm/glm)
add_subdirectory(assimp)
add_subdirectory(imgui)

