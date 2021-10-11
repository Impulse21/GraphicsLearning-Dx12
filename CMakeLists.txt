cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(GraphicsLearning VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakeDependentOption)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty)

add_subdirectory(Shaders)
add_subdirectory(Dx12Core)
add_subdirectory(UnitTests)

target_link_libraries(Dx12Core glfw DirectX-Headers DirectX-Guids Imgui)