cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(Dx12Core VERSION 1.0)

set(dx12_core_include_files
    Include/Dx12Core/Dx12Core.h
    Include/Dx12Core/Application.h
    Include/Dx12Core/Log.h
    Include/Dx12Core/Dx12Common.h
    Include/Dx12Core/d3dx12.h
    Include/Dx12Core/RefCountPtr.h
    Include/Dx12Core/Dx12Factory.h
    Include/Dx12Core/GraphicsDevice.h
    Include/Dx12Core/Dx12Queue.h
    Include/Dx12Core/Dx12DescriptorHeap.h
    Include/Dx12Core/Dx12Resources.h)

set(dx12_core_src_files
    Src/Application.cpp
    Src/Log.cpp
    Src/Dx12Factory.cpp
    Src/GraphicsDevice.cpp
    Src/Dx12Queue.cpp
    Src/Dx12DescriptorHeap.cpp
    Src/Dx12Resources.cpp)

add_library(Dx12Core STATIC 
    ${dx12_core_include_files}
    ${dx12_core_src_files})

target_link_libraries(Dx12Core d3d12.lib dxgi.lib)

target_include_directories(Dx12Core PUBLIC Include ../ThirdParty/spdlog/include ../ThirdParty/glfw)


set_property(TARGET Dx12Core PROPERTY CXX_STANDARD 17)