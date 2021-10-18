
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
	uint32_t InstanceIndex;

	XMFLOAT3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;

	uint32_t AlbedoTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t NormalTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t MetallicTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t RoughnessTexIndex = INVALID_DESCRIPTOR_INDEX;
};

struct SceneInfo
{
	XMFLOAT4X4 ViewProjectionMatrix;
	XMFLOAT3 CameraPosition;
	uint32_t _padding; // I DO NOT understand why this is happening 
	XMFLOAT3 SunDirection;
	uint32_t _padding2;
	XMFLOAT3 SunColour;
	uint32_t IrradnaceMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t PreFilteredEnvMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t BrdfLUT = INVALID_DESCRIPTOR_INDEX;
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

struct InstanceInfo
{
	XMMATRIX WorldMatrix;
};

namespace RootParameters
{
	enum
	{
		DrawInfoCB = 0,
		SceneInfoCB,
		InstanceInfoSB,
		Count
	};
}

namespace SceneType
{
	enum
	{
		Sphere,
		SphereGrid,
		TexturedMaterials,
		Count,
	};

	static const char* Names[] = { "Sphere", "Sphere Grid", "Textured Materials" };
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
	void XM_CALLCONV ComputeMatrices(CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& modelViewProjection);

	std::vector<Vertex> InterleaveVertexData(MeshData const& meshData);

private:
	const size_t SphereGridMaxRows = 6;
	const size_t SphereGridMaxColumns = 6;
	const float SphereGridSpaceing = 1.2;

	// Reserver first entry for the non grid mesh.
	const int SphereGridInstanceDataOffset = 1;

	TextureHandle m_irradanceMap;
	TextureHandle m_prefilteredMap;
	TextureHandle m_brdfLUT;

	std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
	BufferHandle m_vertexbuffer;
	BufferHandle m_indexBuffer;

	MeshData m_sphereMesh;
	BufferHandle m_instanceBuffer;

	GraphicsPipelineHandle m_pipelineState;
	TextureHandle m_depthBuffer;

	XMMATRIX m_viewMatrix = XMMatrixIdentity();
	XMMATRIX m_porjMatrix = XMMatrixIdentity();

	Material m_customMaterial;
	Material m_rustedIronMaterial;

	XMFLOAT3 m_sunDirection = { 1.25, 1.0f, -1.0f};
	XMFLOAT3 m_sunColour = { 1.0f, 1.0f, 1.0f };
	const XMVECTOR m_cameraPositionV = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	const XMFLOAT3 m_cameraPosition = { 0.0f, 0.0f, -10.0f };

	// Settings
	bool m_showRustedIronMateiral = false;
	bool m_enableIBL = true;

	int m_sceneType = SceneType::SphereGrid;


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
	parameterLayout.AddConstantParameter<0, 0>(sizeof(DrawInfo) / 4);
	parameterLayout.AddCBVParameter<1, 0>();
	parameterLayout.AddSRVParameter<0, 0>();
	parameterLayout.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);
	parameterLayout.AddStaticSampler<1, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		);

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

	// Construct instance data
	std::vector<InstanceInfo> instanceData((SphereGridMaxRows* SphereGridMaxColumns) + SphereGridInstanceDataOffset);

	// Reserve
	const XMMATRIX rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	instanceData[0].WorldMatrix = scaleMatrix * rotationMatrix * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	instanceData[0].WorldMatrix = XMMatrixTranspose(instanceData[0].WorldMatrix);
	size_t index = SphereGridInstanceDataOffset;
	for (int iRow = 0; iRow < SphereGridMaxRows; iRow++)
	{
		for (int iCol = 0; iCol < SphereGridMaxColumns; iCol++)
		{

			XMMATRIX translationMatrix =
				XMMatrixTranslation(
					static_cast<float>(iCol - static_cast<int32_t>((SphereGridMaxColumns / 2))) * SphereGridSpaceing,
					(static_cast<float>(iRow - static_cast<int32_t>((SphereGridMaxRows / 2))) * SphereGridSpaceing) + 0.5f,
					0.0f);

			instanceData[index].WorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
			instanceData[index].WorldMatrix = XMMatrixTranspose(instanceData[index].WorldMatrix);
			index++;
		}
	}

	BufferDesc bufferDesc = {};
	bufferDesc.BindFlags = BindFlags::ShaderResource;
	bufferDesc.SizeInBytes = sizeof(InstanceInfo) * instanceData.size();
	bufferDesc.StrideInBytes = sizeof(InstanceInfo);
	bufferDesc.DebugName = L"Instance Buffer";

	this->m_instanceBuffer = GetDevice()->CreateBuffer(bufferDesc);

	copyContext.WriteBuffer<InstanceInfo>(this->m_instanceBuffer, instanceData);
	copyContext.TransitionBarrier(this->m_instanceBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

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

	this->m_prefilteredMap =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\PaperMill_Ruins_E\\PaperMill_RadianceMap.dds",
			copyContext,
			BindFlags::ShaderResource);

	this->m_brdfLUT =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\PaperMill_Ruins_E\\BrdfLut.dds",
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

	static bool showWindow = true;
	ImGui::Begin("Options", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Combo("Scene Type", &this->m_sceneType, SceneType::Names, IM_ARRAYSIZE(SceneType::Names));
	ImGui::NewLine();

	if (ImGui::CollapsingHeader("Material Info"))
	{
		ImGui::Checkbox("Enable IBL", &this->m_enableIBL);

		switch (this->m_sceneType)
		{
		case SceneType::TexturedMaterials:
			break;
		case SceneType::SphereGrid:
			ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&this->m_customMaterial.Albedo));
			ImGui::DragFloat("AO", &this->m_customMaterial.Ao, 0.01f, 0.0f, 1.0f);
			ImGui::TextWrapped("Roughtness increases by Coloumn Left(0.0f) ---> Right(1.0f)");
			ImGui::TextWrapped("Mettalic increates by row Top(0.0f) ---> Bottom(1.0f)");
			break;
		case SceneType::Sphere:
		default:
			ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&this->m_customMaterial.Albedo));
			ImGui::DragFloat("Roughness", &this->m_customMaterial.Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &this->m_customMaterial.Metallic, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("AO", &this->m_customMaterial.Ao, 0.01f, 0.0f, 1.0f);
		}
	}

	ImGui::NewLine();
	
	if (ImGui::CollapsingHeader("Directional Light Parameters"))
	{
		ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&this->m_sunDirection), 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Colour", reinterpret_cast<float*>(&this->m_sunColour));
	}

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


		SceneInfo sceneInfo = {};
		this->ComputeMatrices(
			this->m_viewMatrix,
			this->m_porjMatrix,
			sceneInfo.ViewProjectionMatrix);

		sceneInfo.CameraPosition = this->m_cameraPosition;
		sceneInfo.SunColour = this->m_sunColour;
		sceneInfo.SunDirection = this->m_sunDirection;

		if (this->m_enableIBL)
		{
			sceneInfo.IrradnaceMapTexIndex = this->GetDevice()->GetDescritporIndex(this->m_irradanceMap);
			sceneInfo.PreFilteredEnvMapTexIndex = this->GetDevice()->GetDescritporIndex(this->m_prefilteredMap);
			sceneInfo.BrdfLUT = this->GetDevice()->GetDescritporIndex(this->m_brdfLUT);
		}

		gfxContext.BindDynamicConstantBuffer<SceneInfo>(RootParameters::SceneInfoCB, sceneInfo);
		gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_instanceBuffer);

		DrawInfo drawInfo = {};

		if (this->m_sceneType == SceneType::SphereGrid)
		{
			drawInfo.Albedo = this->m_customMaterial.Albedo;
			drawInfo.Ao = this->m_customMaterial.Ao;
			drawInfo.InstanceIndex = SphereGridInstanceDataOffset;
			for (int iRow = 0; iRow < SphereGridMaxRows; iRow++)
			{
				drawInfo.Metallic = static_cast<float>(iRow) / static_cast<float>(SphereGridMaxRows);
				for (int iCol = 0; iCol < SphereGridMaxColumns; iCol++)
				{
					drawInfo.Roughness = std::clamp(static_cast<float>(iCol) / static_cast<float>(SphereGridMaxColumns), 0.05f, 1.0f);
					gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
					gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());

					drawInfo.InstanceIndex++;
				}
			}
		}
		else
		{
			drawInfo.InstanceIndex = 0;
			
			drawInfo.Albedo = this->m_customMaterial.Albedo;
			drawInfo.AlbedoTexIndex = this->m_customMaterial.AlbedoTexIndex;
			drawInfo.Roughness = this->m_customMaterial.Roughness;
			drawInfo.RoughnessTexIndex = this->m_customMaterial.RoughnessTexIndex;
			drawInfo.Metallic = this->m_customMaterial.Metallic;
			drawInfo.MetallicTexIndex = this->m_customMaterial.MetallicTexIndex;
			drawInfo.Ao = this->m_customMaterial.Ao;
			drawInfo.NormalTexIndex = this->m_customMaterial.NormalTexIndex;

			// based on scene draw a specific way
			gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);

			gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());
		}
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

void XM_CALLCONV PbrDemo::ComputeMatrices(CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& viewProjection)
{
	XMMATRIX m = view * projection;
	XMStoreFloat4x4(&viewProjection, XMMatrixTranspose(m));
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
