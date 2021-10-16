
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

#include "Shaders/PbrDemoVS_compiled.h"
#include "Shaders/PbrDemoPS_compiled.h""

#include <imgui.h>
#include "Dx12Core/ImGui/ImGuiRenderer.h"

#include <DirectXMath.h>

#include "Dx12Core/MeshPrefabs.h"

using namespace Dx12Core;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Positon;
	XMFLOAT3 Normal;
	XMFLOAT3 Colour;
	XMFLOAT2 TexCoord;

};

struct DrawInfo
{
	XMMATRIX WorldMatrix;
	XMMATRIX ModelViewProjectionMatrix;

	XMFLOAT3 CameraPosition;

	XMFLOAT3 SunDirection;
	XMFLOAT3 SunColour;
};

struct Material
{
	XMFLOAT3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;

	uint32_t AlbedoTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t NormalTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t MetallicTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t RoughnessTexIndex = INVALID_DESCRIPTOR_INDEX;
};

namespace RootParameters
{
	enum
	{
		DrawInfoCB = 0,
		MaterialCB,
		Count
	};
}

class PbrDemo : public ApplicationDx12Base
{
public:
	PbrDemo() = default;

protected:
	void LoadContent() override;
	void Update(double elapsedTime) override;
	void Render() override;

private:
	void XM_CALLCONV ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX projection, DrawInfo& drawInfo);

	std::vector<Vertex> InterleaveVertexData(MeshData const& meshData);

private:
	TextureHandle m_irradanceMap;
	std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
	BufferHandle m_vertexbuffer;
	BufferHandle m_indexBuffer;
	MeshData m_sphereMesh;

	GraphicsPipelineHandle m_pipelineState;
	TextureHandle m_depthBuffer;
	XMMATRIX m_meshTransform = XMMatrixIdentity();
	XMMATRIX m_viewMatrix = XMMatrixIdentity();
	XMMATRIX m_porjMatrix = XMMatrixIdentity();

	Material m_customMaterial;
	Material m_rustedIronMaterial;
	bool m_showRustedIronMateiral = false;

	XMFLOAT3 m_sunDirection = { 1.25, 1.0f, 2.0f};
	XMFLOAT3 m_sunColour = { 1.0f, 1.0f, 1.0f };
	const XMVECTOR m_cameraPositionV = XMVectorSet(0, 0, -3, 1);
	const XMFLOAT3 m_cameraPosition = { 0.0f, 0.0f, -3 };
};


CREATE_APPLICATION(PbrDemo)


void PbrDemo::LoadContent()
{
	{
		const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
		this->m_viewMatrix = XMMatrixLookAtLH(this->m_cameraPositionV, focusPoint, upDirection);

		float aspectRatio = 
			this->GetDevice()->GetCurrentSwapChainDesc().Width / static_cast<float>(this->GetDevice()->GetCurrentSwapChainDesc().Height);
		this->m_porjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), aspectRatio, 0.1f, 100.0f);
	}

	{
		TextureDesc desc = {};
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Width = this->GetDevice()->GetCurrentSwapChainDesc().Width;
		desc.Height = this->GetDevice()->GetCurrentSwapChainDesc().Height;
		desc.Dimension = TextureDimension::Texture2D;
		desc.DebugName = "Depth Buffer";
		desc.InitialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.DepthStencil = { 1.0f, 0 };
		desc.OptmizedClearValue = std::make_optional<D3D12_CLEAR_VALUE>(clearValue);

		this->m_depthBuffer = this->GetDevice()->CreateTexture(desc);
	}

	ShaderDesc d = {};
	d.shaderType = ShaderType::Vertex;

	ShaderHandle vs = this->GetDevice()->CreateShader(d, gPbrDemoVS, sizeof(gPbrDemoVS));

	d.shaderType = ShaderType::Pixel;
	ShaderHandle ps = this->GetDevice()->CreateShader(d, gPbrDemoPS, sizeof(gPbrDemoPS));

	// TODO I AM HERE: Add Colour and push Constants
	GraphicsPipelineDesc pipelineDesc = {};
	pipelineDesc.InputLayout =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(this->GetDevice()->GetCurrentSwapChainDesc().Format);
	pipelineDesc.RenderState.DsvFormat = DXGI_FORMAT_D32_FLOAT;

	BindlessShaderParameterLayout bindlessParameterLayout = {};
	bindlessParameterLayout.MaxCapacity = UINT_MAX;
	bindlessParameterLayout.AddParameterSRV(101);
	bindlessParameterLayout.AddParameterSRV(102);

	ShaderParameterLayout parameterLayout = {};
	parameterLayout.AddCBVParameter<0, 0>();
	parameterLayout.AddCBVParameter<1, 0>();
	parameterLayout.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);

	pipelineDesc.UseShaderParameters = true;
	pipelineDesc.ShaderParameters.Binding = &parameterLayout;
	pipelineDesc.ShaderParameters.Bindless = &bindlessParameterLayout;
	pipelineDesc.ShaderParameters.AllowInputLayout();
	this->m_pipelineState = this->GetDevice()->CreateGraphicPipeline(pipelineDesc);

	this->m_imguiRenderer = std::make_unique<ImGuiRenderer>();

	this->m_imguiRenderer->Initialize(this->GetWindow(), this->GetDevice());

	ICommandContext& copyContext = this->GetDevice()->BeginContext();

	this->m_sphereMesh = MeshPrefabs::CreateSphere(1.0f, 16, true);

	LOG_INFO("Interleaving vertex data");
	std::vector<Vertex> vertices = this->InterleaveVertexData(this->m_sphereMesh);
	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(Vertex) * vertices.size();
		bufferDesc.StrideInBytes = sizeof(Vertex);

		this->m_vertexbuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		copyContext.WriteBuffer<Vertex>(this->m_vertexbuffer, vertices);
		copyContext.TransitionBarrier(this->m_vertexbuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint16_t) * this->m_sphereMesh.Indices.size();
		bufferDesc.StrideInBytes = sizeof(uint16_t);

		this->m_indexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<uint16_t>(this->m_indexBuffer, this->m_sphereMesh.Indices);
		copyContext.TransitionBarrier(this->m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	auto albedo =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\rustediron2_basecolor.png",
			copyContext,
			BindFlags::ShaderResource);
	auto normal =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\rustediron2_normal.png",
			copyContext,
			BindFlags::ShaderResource);
	auto roughness =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\rustediron2_roughness.png",
			copyContext,
			BindFlags::ShaderResource);
	auto metallic =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\rustediron2_metallic.png",
			copyContext,
			BindFlags::ShaderResource);

	this->m_irradanceMap =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\PaperMill_Ruins_E\\PaperMill_IrradianceMap.dds",
			copyContext,
			BindFlags::ShaderResource);

	// Load IrradanceMap
	this->GetDevice()->Submit(true);

	this->m_customMaterial = {};
	this->m_customMaterial.Albedo = XMFLOAT3{ 1.0f, 0.0f, 0.0f };
	this->m_customMaterial.Ao = 0.3f;
	this->m_customMaterial.Metallic = 0.4f;
	this->m_customMaterial.Roughness = 1.0f;

	this->m_rustedIronMaterial = {};
	this->m_rustedIronMaterial.AlbedoTexIndex = this->GetDevice()->GetDescritporIndex(albedo);
	this->m_rustedIronMaterial.NormalTexIndex = this->GetDevice()->GetDescritporIndex(normal);
	this->m_rustedIronMaterial.RoughnessTexIndex = this->GetDevice()->GetDescritporIndex(roughness);
	this->m_rustedIronMaterial.MetallicTexIndex = this->GetDevice()->GetDescritporIndex(metallic);
}

void PbrDemo::Update(double elapsedTime)
{
	this->m_imguiRenderer->BeginFrame();


	// Set Matrix data 
	XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	static float i = 0.0f;
	float angle = i;
	i += 10 * elapsedTime;

	const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
	XMMATRIX rotationMatrix = XMMatrixIdentity(); //  XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	this->m_meshTransform = scaleMatrix * rotationMatrix * translationMatrix;


	static bool showWindow = true;
	ImGui::Begin("Options", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::NewLine();
	ImGui::CollapsingHeader("Material Parameters");

	ImGui::Checkbox("Show Rusted Iron Material", &this->m_showRustedIronMateiral);

	if (!m_showRustedIronMateiral)
	{
		ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&this->m_customMaterial.Albedo));
		ImGui::DragFloat("Roughness", &this->m_customMaterial.Roughness, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Metallic", &this->m_customMaterial.Metallic, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("AO", &this->m_customMaterial.Ao, 0.01f, 0.0f, 1.0f);
	}

	ImGui::NewLine();
	ImGui::CollapsingHeader("Directional Light Parameters");
	ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&this->m_sunDirection), 0.01f, -1.0f, 1.0f);
	ImGui::ColorEdit3("Colour", reinterpret_cast<float*>(&this->m_sunColour));
	ImGui::End();
}

void PbrDemo::Render()
{
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Clearing Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 0.0f, 1.0f });

		gfxContext.ClearDepthStencilTexture(this->m_depthBuffer, true, 1.0f, false, 0);
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Draw Triangle");

		const SwapChainDesc& swapChainDesc = this->GetDevice()->GetCurrentSwapChainDesc();

		DrawInfo drawInfo = {};
		this->ComputeMatrices(
			this->m_meshTransform,
			this->m_viewMatrix,
			this->m_porjMatrix,
			drawInfo);

		drawInfo.WorldMatrix = this->m_meshTransform;
		drawInfo.CameraPosition = this->m_cameraPosition;
		drawInfo.SunColour = this->m_sunColour;
		drawInfo.SunDirection = this->m_sunDirection;

		GraphicsState s = {};
		s.VertexBuffer = this->m_vertexbuffer;
		s.IndexBuffer = this->m_indexBuffer;
		s.PipelineState = this->m_pipelineState;
		s.Viewports.push_back(Viewport(swapChainDesc.Width, swapChainDesc.Height));
		s.ScissorRect.push_back(Rect(LONG_MAX, LONG_MAX));

		// TODO: Device should return a handle rather then a week refernce so the context
		// can track the resource.
		s.RenderTargets.push_back(this->GetDevice()->GetCurrentBackBuffer());
		s.DepthStencil = this->m_depthBuffer;

		gfxContext.SetGraphicsState(s);
		gfxContext.BindDynamicConstantBuffer<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);

		if (this->m_showRustedIronMateiral)
		{
			gfxContext.BindDynamicConstantBuffer<Material>(RootParameters::MaterialCB, this->m_rustedIronMaterial);
		}
		else
		{
			gfxContext.BindDynamicConstantBuffer<Material>(RootParameters::MaterialCB, this->m_customMaterial);
		}
		
		gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Draw ImGui");

		this->m_imguiRenderer->Draw(gfxContext, this->GetDevice());
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Transition Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	this->GetDevice()->Submit();
}

void XM_CALLCONV PbrDemo::ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX projection, DrawInfo& drawInfo)
{
	drawInfo.ModelViewProjectionMatrix = model * view * projection;
}

std::vector<Vertex> PbrDemo::InterleaveVertexData(MeshData const& meshData)
{
	std::vector<Vertex> interleavedData(meshData.Positions.size());

	for (int i = 0; i < meshData.Positions.size(); i++)
	{
		interleavedData[i].Positon = meshData.Positions[i];
		interleavedData[i].TexCoord = meshData.TexCoords[i];
		interleavedData[i].Normal = meshData.Normal[i];
		interleavedData[i].Colour = meshData.Colour[i];
	}

	return interleavedData;
}
