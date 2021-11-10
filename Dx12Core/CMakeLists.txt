cmake_minimum_required(VERSION 3.13)


# set the project name and version
project(Dx12Core VERSION 1.0)
    
set(dx12_core_include_files
    Include/Dx12Core/Dx12Core.h
    Include/Dx12Core/Application.h
    Include/Dx12Core/Log.h
    Include/Dx12Core/Dx12Common.h
    Include/Dx12Core/RefCountPtr.h
    Include/Dx12Core/Dx12Factory.h
    Include/Dx12Core/GraphicsDevice.h
    Include/Dx12Core/Dx12Queue.h
    Include/Dx12Core/Dx12CommandAllocatorPool.h
    Include/Dx12Core/ResourceId.h
    Include/Dx12Core/ResouceIdOwner.h
    Include/Dx12Core/ResourceStore.h
    Include/Dx12Core/MeshPrefabs.h
    Include/Dx12Core/ImGui/ImguiRenderer.h)

set(dx12_core_src_files
    Src/Application.cpp
    Src/Log.cpp
    Src/Dx12Factory.cpp
    Src/GraphicsDevice.cpp
    Src/Dx12Queue.cpp
    Src/Dx12DescriptorHeap.h
    Src/Dx12DescriptorHeap.cpp
    Src/Dx12CommandAllocatorPool.cpp
    Src/Dx12Resources.h
    Src/Dx12CommandContext.h
    Src/Dx12CommandContext.cpp
    Src/DescriptorAllocation.h
    Src/DescriptorAllocation.cpp
    Src/Dx12UploadBuffer.h
    Src/Dx12UploadBuffer.cpp
    Src/ResourceStore.cpp
    Src/MeshPrefabs.cpp
    Src/ImGui/imgui_impl_glfw.h
    Src/ImGui/imgui_impl_glfw.cpp
    Src/ImGui/ImguiRenderer.cpp
    Src/DescriptorHeap.h
    Src/DescriptorHeap.cpp)

add_library(Dx12Core STATIC 
    ${dx12_core_include_files}
    ${dx12_core_src_files})

target_link_libraries(
    Dx12Core
    d3d12.lib
    dxgi.lib
    dxguid.lib
    DirectXTex)

target_include_directories(
    Dx12Core PUBLIC 
    Include 
    Src 
    ../ThirdParty/spdlog/include 
    ../ThirdParty/glfw 
    ../ThirdParty/DirectXTex/DirectXTex 
    Dx12Headers 
    ../ThirdParty/imgui 
    ../Shaders/Include)


set_property(TARGET Dx12Core PROPERTY CXX_STANDARD 17)