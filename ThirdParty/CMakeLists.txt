
# IMGUI

set (imgui_include_files
        imgui/imconfig.h
        imgui/imgui.h
        imgui/imgui_internal.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h)
set (imgui_src_files
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_tables.cpp)

add_library(Imgui STATIC ${imgui_include_files} ${imgui_src_files})
set_property(TARGET Imgui PROPERTY CXX_STANDARD 17)

add_subdirectory(glfw)
add_subdirectory(DirectXTex)
add_subdirectory(DirectX-Headers)

set_target_properties(glfw PROPERTIES FOLDER "ThirdParty")
set_target_properties(DirectXTex PROPERTIES FOLDER "ThirdParty")
set_target_properties(DirectX-Guids PROPERTIES FOLDER "ThirdParty")
set_target_properties(Imgui PROPERTIES FOLDER "ThirdParty")


SET(BUILD_TOOLS OFF CACHE BOOL "Build tex command-line tools")
SET(BUILD_DX11 OFF CACHE BOOL "Build with DirectX11 Runtime support")
SET(BUILD_DX12 OFF CACHE BOOL "Build with DirectX12 Runtime support")

