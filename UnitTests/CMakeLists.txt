cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(UnitTests VERSION 1.0)

# set the project name and version
add_executable(TriangleApp Src/Triangle.cpp)

target_link_libraries(TriangleApp d3d12.lib dxgi.lib Shaders Dx12Core)
target_include_directories(TriangleApp PUBLIC ../Dx12Core/Include ../ThirdParty/spdlog/include ../ThirdParty/imgui)



set_target_properties(TriangleApp PROPERTIES FOLDER "UnitTests")