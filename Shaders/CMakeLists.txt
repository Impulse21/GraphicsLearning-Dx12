cmake_minimum_required(VERSION 3.13)

# set the project name and version
project(Shaders VERSION 1.0)

# Shader Projects
set(shader_source_vs
    Src/TriangleVS.hlsl
    Src/CubeVS.hlsl
    Src/TexturedCubeVS.hlsl
    Src/BindlessExampleVS.hlsl
    Src/ImGuiVS.hlsl
    Src/PbrDemoVS.hlsl
    Src/SkyboxVS.hlsl)
    
set(shader_source_ps
    Src/TrianglePS.hlsl
    Src/CubePS.hlsl
    Src/TexturedCubePS.hlsl
    Src/BindlessExamplePS.hlsl
    Src/ImGuiPS.hlsl
    Src/PbrDemoPS.hlsl
    Src/SkyboxPS.hlsl)

set(shader_headers
    Src/Defines.hlsli
    Src/BindlessExample.hlsli
    Src/BRDFFunctions.hlsli)


set(shader_source_includes
    Include/Shaders/TriangleVS_compiled.h
    Include/Shaders/TrianglePS_compiled.h
    Include/Shaders/CubeVS_compiled.h
    Include/Shaders/CubePS_compiled.h
    Include/Shaders/TexturedCubeVS_compiled.h
    Include/Shaders/TexturedCubePS_compiled.h
    Include/Shaders/BindlessExampleVS_compiled.h
    Include/Shaders/BindlessExamplePS_compiled.h
    Include/Shaders/ImGuiVS_compiled.h
    Include/Shaders/ImGuiPS_compiled.h
    Include/Shaders/PbrDemoVS_compiled.h
    Include/Shaders/PbrDemoPS_compiled.h
    Include/Shaders/SkyboxVS_compiled.h
    Include/Shaders/SkyboxPS_compiled.h)

# first is extra debug flags
set_source_files_properties(${shader_source_vs} PROPERTIES VS_SHADER_FLAGS "/Fh \"${PROJECT_SOURCE_DIR}/Include/Shaders/%(Filename)_compiled.h\" /Vn \"g%(Filename)\" /Zi /Od /Qembed_debug" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 6.5 VS_SHADER_ENTRYPOINT main)
#set_source_files_properties(${shader_source_vs} PROPERTIES VS_SHADER_FLAGS "/Fh \"${PROJECT_SOURCE_DIR}/Include/Shaders/%(Filename)_compiled.h\" /Vn \"g%(Filename)\" /Zi" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 6.5 VS_SHADER_ENTRYPOINT main)

# first is extra debug flags
#set_source_files_properties(${shader_source_ps} PROPERTIES VS_SHADER_FLAGS "/Fh \"${PROJECT_SOURCE_DIR}/Include/Shaders/%(Filename)_compiled.h\" /Vn \"g%(Filename)\" /Zi /Od /Qembed_debug" VS_SHADER_TYPE Pixel  VS_SHADER_MODEL 6.5 VS_SHADER_ENTRYPOINT main)
set_source_files_properties(${shader_source_ps} PROPERTIES VS_SHADER_FLAGS "/Fh \"${PROJECT_SOURCE_DIR}/Include/Shaders/%(Filename)_compiled.h\" /Vn \"g%(Filename)\" /Zi" VS_SHADER_TYPE Pixel  VS_SHADER_MODEL 6.5 VS_SHADER_ENTRYPOINT main)

add_library(Shaders INTERFACE 
    ${shader_source_vs}
    ${shader_source_ps}
    ${shader_headers}
    ${shader_source_includes})
    