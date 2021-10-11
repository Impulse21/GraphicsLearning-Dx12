#include "ImGuiRenderer.h"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

using namespace ImGui;

Dx12Core::ImGuiRenderer::~ImGuiRenderer()
{
	if (this->m_imguiContext)
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext(this->m_imguiContext);
		this->m_imguiContext = nullptr;
	}
}
void Dx12Core::ImGuiRenderer::Initialize(GLFWwindow* glfwWindow, IGraphicsDevice* device)
{
	this->m_imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(this->m_imguiContext);

	if (!ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(glfwWindow), false))
	{
		LOG_CORE_ERROR("Failed to initalize IMGUI Window");
		throw std::runtime_error("Error");
	}


	ImGuiIO& io = ImGui::GetIO();
	io.FontAllowUserScaling = true;

	unsigned char* pixelData = nullptr;
	int width;
	int height;

	io.Fonts->GetTexDataAsRGBA32(&pixelData, &width, &height);

	TextureDesc desc = {};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Width = width;
	desc.Height = height;
	desc.Dimension = TextureDimension::Texture2D;
	desc.DebugName = "ImGui Texture";
	desc.InitialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	desc.Bindings = BindFlags::ShaderResource;
	desc.MipLevels = 1;

	this->m_fontTexture = device->CreateTexture(desc);

	ICommandContext& copyContext = device->BeginContext();

	D3D12_SUBRESOURCE_DATA subresource = {};
	subresource.RowPitch = width * 4;
	subresource.SlicePitch = subresource.RowPitch * height;
	subresource.pData = pixelData;

	copyContext.WriteTexture(this->m_fontTexture, 0, 1, &subresource);
	device->Submit();

	this->CreatePso();
}

void Dx12Core::ImGuiRenderer::CreatePso()
{
}
