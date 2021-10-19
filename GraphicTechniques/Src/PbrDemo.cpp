
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
	XMFLOAT4 Tangent;
	XMFLOAT4 BiTangent;

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
	uint32_t AoTexIndex = INVALID_DESCRIPTOR_INDEX;
};

struct InstanceInfo
{
	XMMATRIX WorldMatrix;
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

struct EnvInfo
{
	TextureHandle SkyBox;
	TextureHandle IrradanceMap;
	TextureHandle PrefilteredMap;
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
	uint32_t AoTexIndex = INVALID_DESCRIPTOR_INDEX;
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



static const char* RenderModelNames[] = { "Disabled", "Enviroment", "Irradiance", "PF Radiance" };

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

	void LoadMaterials(ICommandContext& context);
	void LoadMaterial(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& albedoPath,
		std::string const& bumpMapPath,
		std::string const& roughnessPath,
		std::string const& metallicPath,
		Material& outMaterial);

	void LoadMaterial(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& albedoPath,
		std::string const& bumpMapPath,
		std::string const& roughnessPath,
		std::string const& metallicPath,
		std::string const& aoPath,
		Material& outMaterial);

	void LoadEnviroments(ICommandContext& context);
	void LoadEnvData(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& skyboxPath,
		std::string const& irradanceMapPath,
		std::string const& prefilteredRadanceMapPath,
		EnvInfo& outEnviroment);

	void CreateRenderPass();
	void CreateRenderPassSkybox();

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

	std::vector<EnvInfo> m_enviroments;

	XMFLOAT3 m_sunDirection = { 1.25, 1.0f, -1.0f};
	XMFLOAT3 m_sunColour = { 1.0f, 1.0f, 1.0f };


	const XMVECTOR m_focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR m_upDirection = XMVectorSet(0, 1, 0, 0);
	const XMVECTOR m_cameraClose = XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);
	const XMVECTOR m_cameraFar = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	XMVECTOR m_cameraPosition = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

	int m_sceneType = SceneType::SphereGrid;

	struct MaterialSettings
	{
		std::vector<const char*> MaterialNames;
		int SelectedMaterialId = 0;
		bool DisableIbl = false;

	} m_materialSettings;

	std::vector<Material> m_material;

	struct EnviromentSettings
	{
		std::vector<const char*> EnvNames;
		int SelectedEnvNameId = 0;

		enum SkyboxRenderMode
		{
			Disabled,
			Enviroment,
			Irradiance,
			PrefilterdRadiance,
		};

		int RenderMode = SkyboxRenderMode::Enviroment;
	} m_enviromentSettings;
};


CREATE_APPLICATION(PbrDemo)


void PbrDemo::LoadContent()
{
	{
		this->m_viewMatrix = XMMatrixLookAtLH(this->m_cameraPosition, this->m_focusPoint, this->m_upDirection);

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

	this->CreateRenderPass();

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


	this->LoadMaterials(copyContext);
	this->LoadEnviroments(copyContext);

	this->m_brdfLUT =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\IBL\\BrdfLut.dds",
			copyContext,
			BindFlags::ShaderResource);

	// Load IrradanceMap
	this->GetDevice()->Submit(true);
}

void PbrDemo::Update(double elapsedTime)
{
	this->m_imguiRenderer->BeginFrame();

	static bool showWindow = true;
	ImGui::Begin("Options", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Combo("Scene Type", &this->m_sceneType, SceneType::Names, IM_ARRAYSIZE(SceneType::Names));
	ImGui::NewLine();

	this->m_cameraPosition = this->m_sceneType == SceneType::SphereGrid
		? this->m_cameraFar
		: this->m_cameraClose;
	this->m_viewMatrix = XMMatrixLookAtLH(this->m_cameraPosition, this->m_focusPoint, this->m_upDirection);

	if (ImGui::CollapsingHeader("Enviroment Settings"))
	{
		ImGui::Combo("Skybox Render Mode", &this->m_enviromentSettings.RenderMode, RenderModelNames, IM_ARRAYSIZE(RenderModelNames));

		if (this->m_enviromentSettings.RenderMode != EnviromentSettings::SkyboxRenderMode::Disabled)
		{
			ImGui::Combo(
				"Skybox",
				&this->m_enviromentSettings.SelectedEnvNameId,
				this->m_enviromentSettings.EnvNames.data(),
				this->m_enviromentSettings.EnvNames.size());
		}

	}
	if (ImGui::CollapsingHeader("Material Settings"))
	{
		ImGui::Checkbox("Disable IBL", &this->m_materialSettings.DisableIbl);

		switch (this->m_sceneType)
		{
		case SceneType::SphereGrid:
		{
			this->m_materialSettings.SelectedMaterialId = 0;
			auto& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];
			ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&selectedMaterial.Albedo));
			ImGui::DragFloat("AO", &selectedMaterial.Ao, 0.01f, 0.0f, 1.0f);
			ImGui::TextWrapped("Roughtness increases by Coloumn Left(0.0f) ---> Right(1.0f)");
			ImGui::TextWrapped("Mettalic increates by row Top(0.0f) ---> Bottom(1.0f)");
		}
			break;

		case SceneType::TexturedMaterials:
		case SceneType::Sphere:
		{

			ImGui::Combo(
				"Material Type",
				&this->m_materialSettings.SelectedMaterialId,
				this->m_materialSettings.MaterialNames.data(),
				this->m_materialSettings.MaterialNames.size());

			auto& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];

			if (selectedMaterial.AlbedoTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&selectedMaterial.Albedo));
			}

			if (selectedMaterial.RoughnessTexIndex== INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("Roughness", &selectedMaterial.Roughness, 0.01f, 0.0f, 1.0f);
			}

			if (selectedMaterial.MetallicTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("Metallic", &selectedMaterial.Metallic, 0.01f, 0.0f, 1.0f);
			}

			if (selectedMaterial.AoTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("AO", &selectedMaterial.Ao, 0.01f, 0.0f, 1.0f);
			}
		}
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
		ScopedMarker m = gfxContext.BeginScropedMarker("Main Render Pass");

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

		XMStoreFloat3(&sceneInfo.CameraPosition, this->m_cameraPosition);
		sceneInfo.SunColour = this->m_sunColour;
		sceneInfo.SunDirection = this->m_sunDirection;

		if (!this->m_materialSettings.DisableIbl)
		{
			auto& envInfo = this->m_enviroments[this->m_enviromentSettings.SelectedEnvNameId];
			sceneInfo.IrradnaceMapTexIndex = this->GetDevice()->GetDescritporIndex(envInfo.IrradanceMap);
			sceneInfo.PreFilteredEnvMapTexIndex = this->GetDevice()->GetDescritporIndex(envInfo.PrefilteredMap);
			sceneInfo.BrdfLUT = this->GetDevice()->GetDescritporIndex(this->m_brdfLUT);
		}

		gfxContext.BindDynamicConstantBuffer<SceneInfo>(RootParameters::SceneInfoCB, sceneInfo);
		gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_instanceBuffer);

		DrawInfo drawInfo = {};

		Material& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];
		drawInfo.Albedo = selectedMaterial.Albedo;
		drawInfo.AlbedoTexIndex = selectedMaterial.AlbedoTexIndex;
		drawInfo.Roughness = selectedMaterial.Roughness;
		drawInfo.RoughnessTexIndex = selectedMaterial.RoughnessTexIndex;
		drawInfo.Metallic = selectedMaterial.Metallic;
		drawInfo.MetallicTexIndex = selectedMaterial.MetallicTexIndex;
		drawInfo.Ao = selectedMaterial.Ao;
		drawInfo.AoTexIndex = selectedMaterial.AoTexIndex;
		drawInfo.NormalTexIndex = selectedMaterial.NormalTexIndex;
		
		if (this->m_sceneType == SceneType::SphereGrid)
		{
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
			gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
			gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());
		}
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Render Skybox");
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Draw ImGui UI");

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
		interleavedData[i].Tangent = meshData.Tangents[i];
		interleavedData[i].BiTangent= meshData.BiTangents[i];
	}

	return interleavedData;
}

void PbrDemo::LoadMaterials(ICommandContext& context)
{
	// Load Custom Material
	{
		Material& material = m_material.emplace_back();
		material.Albedo = XMFLOAT3{ 1.0f, 0.0f, 0.0f };
		material.Ao = 0.3f;
		material.Metallic = 0.4f;
		material.Roughness = 1.0f;

		this->m_materialSettings.MaterialNames.emplace_back("Custom Material");
	}

	const std::string BaseDir = "Assets\\Textures\\\Materials\\";
	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"rustediron2\\rustediron2_basecolor.png",
			"rustediron2\\rustediron2_normal.png",
			"rustediron2\\rustediron2_roughness.png",
			"rustediron2\\rustediron2_metallic.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("rusted iron");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"gray-granite-flecks\\gray-granite-flecks-albedo.png",
			"gray-granite-flecks\\gray-granite-flecks-Normal-dx.png",
			"gray-granite-flecks\\gray-granite-flecks-Roughness.png",
			"gray-granite-flecks\\gray-granite-flecks-Metallic.png",
			"gray-granite-flecks\\gray-granite-flecks-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("gray granite flecks");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"redbricks2b\\redbricks2b-albedo.png",
			"redbricks2b\\redbricks2b-normal.png",
			"redbricks2b\\redbricks2b-rough.png",
			"redbricks2b\\redbricks2b-metalness.png",
			"redbricks2b\\redbricks2b-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("red bricks");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"worn-shiny-metal\\worn-shiny-metal-albedo.png",
			"worn-shiny-metal\\worn-shiny-metal-Normal-dx.png",
			"worn-shiny-metal\\worn-shiny-metal-Roughness.png",
			"worn-shiny-metal\\worn-shiny-metal-Metallic.png",
			"worn-shiny-metal\\worn-shiny-metal-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("stworn shiny metal");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"outdoor-polyester-fabric1\\outdoor-polyester-fabric_albedo.png",
			"outdoor-polyester-fabric1\\outdoor-polyester-fabric_normal-dx.png",
			"outdoor-polyester-fabric1\\outdoor-polyester-fabric_roughness.png",
			"outdoor-polyester-fabric1\\outdoor-polyester-fabric_metallic.png",
			"outdoor-polyester-fabric1\\outdoor-polyester-fabric_ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("polyester fabric1");
	}
}

void PbrDemo::LoadMaterial(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& albedoPath,
	std::string const& bumpMapPath,
	std::string const& roughnessPath,
	std::string const& metallicPath,
	Material& outMaterial)
{
	auto albedo =
		this->GetTextureStore()->Load(
			basePath + albedoPath,
			context,
			BindFlags::ShaderResource);
	auto normal =
		this->GetTextureStore()->Load(
			basePath + bumpMapPath,
			context,
			BindFlags::ShaderResource);
	auto roughness =
		this->GetTextureStore()->Load(
			basePath + roughnessPath,
			context,

			BindFlags::ShaderResource);
	auto metallic =
		this->GetTextureStore()->Load(
			basePath + metallicPath,
			context,
			BindFlags::ShaderResource);


	outMaterial.AlbedoTexIndex = this->GetDevice()->GetDescritporIndex(albedo);
	outMaterial.NormalTexIndex = this->GetDevice()->GetDescritporIndex(normal);
	outMaterial.RoughnessTexIndex = this->GetDevice()->GetDescritporIndex(roughness);
	outMaterial.MetallicTexIndex = this->GetDevice()->GetDescritporIndex(metallic);
}

void PbrDemo::LoadMaterial(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& albedoPath,
	std::string const& bumpMapPath,
	std::string const& roughnessPath, 
	std::string const& metallicPath,
	std::string const& aoPath,
	Material& outMaterial)
{
	this->LoadMaterial(
		context,
		basePath,
		albedoPath,
		bumpMapPath,
		roughnessPath,
		metallicPath,
		outMaterial);

	auto ao =
		this->GetTextureStore()->Load(
			basePath + aoPath,
			context,
			BindFlags::ShaderResource);

	outMaterial.AoTexIndex = this->GetDevice()->GetDescritporIndex(ao);
}

void PbrDemo::LoadEnviroments(ICommandContext& context)
{
	const std::string BaseDir = "Assets\\Textures\\\IBL\\";
	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"PaperMill_Ruins_E\\PaperMill_Skybox.dds",
			"PaperMill_Ruins_E\\PaperMill_IrradianceMap.dds",
			"PaperMill_Ruins_E\\PaperMill_RadianceMap.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Paper Mill Ruins");
	}

	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"Serpentine_Valley\\output_skybox.dds",
			"Serpentine_Valley\\output_irradiance.dds",
			"Serpentine_Valley\\output_radiance.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Serpentine Valley");
	}

	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"cmftStudio\\output_skybox.dds",
			"cmftStudio\\output_irradiance.dds",
			"cmftStudio\\output_radiance.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Cmft Studio");
	}
}

void PbrDemo::LoadEnvData(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& skyboxPath,
	std::string const& irradanceMapPath,
	std::string const& prefilteredRadanceMapPath,
	EnvInfo& outEnviroment)
{
	outEnviroment.SkyBox =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\IBL\\PaperMill_Ruins_E\\PaperMill_RadianceMap.dds",
			context,
			BindFlags::ShaderResource);

	outEnviroment.IrradanceMap =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\IBL\\PaperMill_Ruins_E\\PaperMill_IrradianceMap.dds",
			context,
			BindFlags::ShaderResource);

	outEnviroment.PrefilteredMap =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\IBL\\PaperMill_Ruins_E\\PaperMill_IrradianceMap.dds",
			context,
			BindFlags::ShaderResource);
}

void PbrDemo::CreateRenderPass()
{
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
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
}
