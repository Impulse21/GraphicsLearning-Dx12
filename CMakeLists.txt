cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(GraphicsLearning VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakeDependentOption)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/glfw)
add_subdirectory(ThirdParty/DirectXTex)
add_subdirectory(ThirdParty/DirectX-Headers)

add_subdirectory(Shaders)
add_subdirectory(Dx12Core)
target_link_libraries(Dx12Core glfw DirectX-Headers DirectX-Guids)

add_subdirectory(UnitTests)

set_target_properties(glfw PROPERTIES FOLDER "ThirdParty")
set_target_properties(DirectXTex PROPERTIES FOLDER "ThirdParty")
set_target_properties(DirectX-Guids PROPERTIES FOLDER "ThirdParty")

SET(BUILD_TOOLS OFF CACHE BOOL "Build tex command-line tools")
SET(BUILD_DX11 OFF CACHE BOOL "Build with DirectX11 Runtime support")
SET(BUILD_DX12 OFF CACHE BOOL "Build with DirectX12 Runtime support")