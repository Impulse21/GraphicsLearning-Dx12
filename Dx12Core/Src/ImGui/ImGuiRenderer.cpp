#include "Dx12Core/ImGui/ImGuiRenderer.h"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

#include "Shaders/ImGuiVS_compiled.h"
#include "Shaders/ImGuiPS_compiled.h"
#include <DirectXMath.h>

using namespace ImGui;
using namespace DirectX;

struct DrawInfo
{
	XMMATRIX Mvp;
	uint32_t TextureIndex;
};

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
	desc.InitialState = D3D12_RESOURCE_STATE_COPY_DEST;
	desc.Bindings = BindFlags::ShaderResource;
	desc.MipLevels = 1;

	this->m_fontTexture = device->CreateTexture(desc);

	ICommandContext& copyContext = device->BeginContext();

	D3D12_SUBRESOURCE_DATA subresource = {};
	subresource.RowPitch = width * 4;
	subresource.SlicePitch = subresource.RowPitch * height;
	subresource.pData = pixelData;

	copyContext.WriteTexture(this->m_fontTexture, 0, 1, &subresource);
	copyContext.TransitionBarrier(this->m_fontTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// copyContext.WriteTexture(this->m_fontTexture, 0, 0, pixelData, width * 4, 0);
	device->Submit();

	this->CreatePso(device);
}

void Dx12Core::ImGuiRenderer::BeginFrame()
{
	ImGui::SetCurrentContext(this->m_imguiContext);
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Dx12Core::ImGuiRenderer::Draw(ICommandContext& commandContext, IGraphicsDevice* device)
{
    ImGui::SetCurrentContext(this->m_imguiContext);
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* drawData = ImGui::GetDrawData();

    // Check if there is anything to render.
    if (!drawData || drawData->CmdListsCount == 0)
    {
        return;
    }

    ImVec2 displayPos = drawData->DisplayPos;


    // Set root arguments.
    DrawInfo drawInfo = {};

    //    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicRH( drawData->DisplaySize.x, drawData->DisplaySize.y, 0.0f, 1.0f );
    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

    drawInfo.Mvp =
    {
        { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
        { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
        { 0.0f,         0.0f,           0.5f,       0.0f },
        { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
    };


    GraphicsState s = {};
    s.PipelineState = this->m_pso;
	s.Viewports.push_back(Viewport(device->GetCurrentSwapChainDesc().Width, device->GetCurrentSwapChainDesc().Height));
	s.ScissorRect.push_back(Rect(LONG_MAX, LONG_MAX));
	s.RenderTargets.push_back(device->GetCurrentBackBuffer());
	commandContext.SetGraphicsState(s);

    // drawInfo.Mvp = glm::transpose(drawInfo.Mvp);

    // Invalid Texture index
    drawInfo.TextureIndex = device->GetDescritporIndex(this->m_fontTexture);
	commandContext.BindGraphics32BitConstants(0, drawInfo);


    // commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const DXGI_FORMAT indexFormat = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    // It may happen that ImGui doesn't actually render anything. In this case,
    // any pending resource barriers in the commandList will not be flushed (since 
    // resource barriers are only flushed when a draw command is executed).
    // In that case, manually flushing the resource barriers will ensure that
    // they are properly flushed before exiting this function.
    // commandList->FlushResourceBarriers();

    for (int i = 0; i < drawData->CmdListsCount; ++i)
    {
        const ImDrawList* drawList = drawData->CmdLists[i];

		commandContext.BindDynamicVertexBuffer(0, drawList->VtxBuffer.size(), sizeof(ImDrawVert), drawList->VtxBuffer.Data);
		commandContext.BindDynamicIndexBuffer(drawList->IdxBuffer.size(), indexFormat, drawList->IdxBuffer.Data);

        int indexOffset = 0;
        for (int j = 0; j < drawList->CmdBuffer.size(); ++j)
        {
            const ImDrawCmd& drawCmd = drawList->CmdBuffer[j];
            if (drawCmd.UserCallback)
            {
                drawCmd.UserCallback(drawList, &drawCmd);
            }
            else
            {
                ImVec4 clipRect = drawCmd.ClipRect;
				Rect scissorRect(
					static_cast<LONG>(clipRect.x - displayPos.x),
					static_cast<LONG>(clipRect.z - displayPos.x),
					static_cast<LONG>(clipRect.y - displayPos.y),
					static_cast<LONG>(clipRect.w - displayPos.y));

                if (scissorRect.MaxX - scissorRect.MinX > 0.0f &&
                    scissorRect.MaxY - scissorRect.MinY > 0.0)
                {
					commandContext.BindScissorRects({ scissorRect });
                    commandContext.DrawIndexed(drawCmd.ElemCount, 1, indexOffset);
                }
            }
            indexOffset += drawCmd.ElemCount;
        }
    }

    ImGui::EndFrame();
}

void Dx12Core::ImGuiRenderer::CreatePso(IGraphicsDevice* device)
{

    ShaderDesc shaderDesc = {};
    shaderDesc.shaderType = ShaderType::Vertex;
    shaderDesc.debugName = "IMGUI Vertex Shader";

    ShaderHandle vs = device->CreateShader(shaderDesc, gImGuiVS, sizeof(gImGuiVS));

    shaderDesc.shaderType = ShaderType::Pixel;
    shaderDesc.debugName = "IMGUI Pixel Shader";

    ShaderHandle ps = device->CreateShader(shaderDesc, gImGuiPS, sizeof(gImGuiPS));

    GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.InputLayout = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };


	ShaderParameterLayout parameterLayout = {};
	parameterLayout.AddConstantParameter<0, 0>(sizeof(DrawInfo) / 4);
	parameterLayout.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);

	BindlessShaderParameterLayout bindlessParameterLayout = {};
	bindlessParameterLayout.MaxCapacity = UINT_MAX;
	bindlessParameterLayout.AddParameterSRV(100);

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(device->GetCurrentSwapChainDesc().Format);
	// pipelineDesc.RenderState.DsvFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineDesc.UseShaderParameters = true;
	pipelineDesc.ShaderParameters.Binding = &parameterLayout;
	pipelineDesc.ShaderParameters.Bindless = &bindlessParameterLayout;
	pipelineDesc.ShaderParameters.AllowInputLayout();

	CD3DX12_BLEND_DESC blendState = {};
	blendState.RenderTarget[0].BlendEnable = true;
	blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	pipelineDesc.RenderState.BlendState = &blendState;

	// Most of these are defaults. Consider using the explcity constructor
	CD3DX12_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState.CullMode = D3D12_CULL_MODE_NONE; // Not Default
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	rasterizerState.ForcedSampleCount = 0;

	pipelineDesc.RenderState.RasterizerState = &rasterizerState;

	CD3DX12_DEPTH_STENCIL_DESC depthState = {};
	depthState.DepthEnable = false;
	depthState.StencilEnable = false;

	pipelineDesc.RenderState.DepthStencilState = &depthState;

	this->m_pso = device->CreateGraphicPipeline(pipelineDesc);
}
