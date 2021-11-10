cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(GraphicTechniques VERSION 1.0)

# set the project name and version
add_executable(PbrDemo Src/PbrDemo.cpp)

target_link_libraries(PbrDemo d3d12.lib dxgi.lib Shaders Dx12Core DirectXTex)
target_include_directories(PbrDemo PUBLIC ../Dx12Core/Include ../ThirdParty/spdlog/include ../ThirdParty/imgui ../Shaders/Include ../ThirdParty/DirectXTex/DirectXTex ../ThridParty/Imgui)

set_target_properties(PbrDemo PROPERTIES FOLDER "GraphicTechniques")

set_target_properties(PbrDemo PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	VS_DEBUGGER_COMMAND_ARGUMENTS "Assets")

	

# set the project name and version
add_executable(DeferredRenderer Src/DeferredRenderer.cpp)

target_link_libraries(DeferredRenderer d3d12.lib dxgi.lib Shaders Dx12Core DirectXTex)
target_include_directories(
	DeferredRenderer PUBLIC 
	../Dx12Core/Include
	../ThirdParty/spdlog/include
	../ThirdParty/imgui
	../Shaders/Include
	../ThirdParty/DirectXTex/DirectXTex
	../ThridParty/Imgui)

set_target_properties(DeferredRenderer PROPERTIES FOLDER "GraphicTechniques")

set_target_properties(DeferredRenderer PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	VS_DEBUGGER_COMMAND_ARGUMENTS "Assets")