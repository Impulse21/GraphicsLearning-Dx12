cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(UnitTests VERSION 1.0)

# set the project name and version
add_executable(TriangleApp Src/Triangle.cpp)

target_link_libraries(TriangleApp d3d12.lib dxgi.lib Shaders Dx12Core DirectXTex)
target_include_directories(TriangleApp PUBLIC ../Dx12Core/Include ../ThirdParty/spdlog/include ../ThirdParty/imgui ../Shaders/Include ../ThirdParty/DirectXTex/DirectXTex)

set_target_properties(TriangleApp PROPERTIES FOLDER "UnitTests")

# set the project name and version
add_executable(CubeApp Src/Cube.cpp)

target_link_libraries(CubeApp d3d12.lib dxgi.lib Shaders Dx12Core DirectXTex)
target_include_directories(CubeApp PUBLIC ../Dx12Core/Include ../ThirdParty/spdlog/include ../ThirdParty/imgui ../Shaders/Include ../ThirdParty/DirectXTex/DirectXTex)

set_target_properties(CubeApp PROPERTIES FOLDER "UnitTests")


# set the project name and version
add_executable(TexturedCubeApp Src/TexturedCube.cpp)

target_link_libraries(TexturedCubeApp d3d12.lib dxgi.lib Shaders Dx12Core DirectXTex)
target_include_directories(TexturedCubeApp PUBLIC ../Dx12Core/Include ../ThirdParty/spdlog/include ../ThirdParty/imgui ../Shaders/Include ../ThirdParty/DirectXTex/DirectXTex)

set_target_properties(TexturedCubeApp PROPERTIES FOLDER "UnitTests")

