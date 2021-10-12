
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

#include "Shaders/TexturedCubeVS_compiled.h"
#include "Shaders/TexturedCubePS_compiled.h"

#include <DirectXMath.h>

using namespace Dx12Core;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Positon;
	XMFLOAT3 Colour;
	XMFLOAT2 TexCoord;

	Vertex(XMVECTOR pos, XMVECTORF32 texCoord, XMFLOAT3 colour)
		: Colour(colour)
	{
		XMStoreFloat3(&Positon, pos);
		XMStoreFloat2(&TexCoord, texCoord);
	}
};

struct DrawInfo
{
	XMMATRIX ModelViewProjectionMatrix;
};

struct MaterialInfo
{
	uint32_t albedoIndex;
};

namespace RootParameters
{
	enum
	{
		PushConstant = 0,
		DrawInfoCB,
		TextureSRV,
		Sampler,
		Count
	};
}

// Helper for flipping winding of geometric primitives for LH vs. RH coords
static void ReverseWinding(std::vector<uint16_t>& indices, std::vector<Vertex>& vertices)
{
	assert((indices.size() % 3) == 0);
	for (auto it = indices.begin(); it != indices.end(); it += 3)
	{
		std::swap(*it, *(it + 2));
	}
}

class TexturedCubeApp : public ApplicationDx12Base
{
public:
	TexturedCubeApp() = default;

protected:
	void LoadContent() override;
	void Update(double elapsedTime) override;
	void Render() override;

	void CreateCube(float size, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, bool rhsCoord = false);

private:
	void XM_CALLCONV ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX projection, DrawInfo& drawInfo);

private:
	GraphicsPipelineHandle m_pipelineState;
	TextureHandle m_depthBuffer;
	BufferHandle m_vertexbuffer;
	BufferHandle m_indexBuffer;
	XMMATRIX m_cubeWorldTransform = XMMatrixIdentity();
	XMMATRIX m_viewMatrix = XMMatrixIdentity();
	XMMATRIX m_porjMatrix = XMMatrixIdentity();

	TextureHandle m_cubeTexture;

	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;
};


CREATE_APPLICATION(TexturedCubeApp)


void TexturedCubeApp::LoadContent()
{
	{
		const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
		const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
		this->m_viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

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

	ShaderHandle vs = this->GetDevice()->CreateShader(d, gTexturedCubeVS, sizeof(gTexturedCubeVS));

	d.shaderType = ShaderType::Pixel;
	ShaderHandle ps = this->GetDevice()->CreateShader(d, gTexturedCubePS, sizeof(gTexturedCubePS));

	// TODO I AM HERE: Add Colour and push Constants
	GraphicsPipelineDesc pipelineDesc = {};
	pipelineDesc.InputLayout =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(this->GetDevice()->GetCurrentSwapChainDesc().Format);
	pipelineDesc.RenderState.DsvFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineDesc.RootSignatureDesc.AllowInputLayout();
	pipelineDesc.RootSignatureDesc.Add32BitConstants<0, 0>(sizeof(MaterialInfo) / 4);
	pipelineDesc.RootSignatureDesc.AddConstantBufferView<1, 0>();

	// -- SRV Table Set-up ---
	DescriptorTableDesc srvTable;
	{
		constexpr D3D12_DESCRIPTOR_RANGE_FLAGS flags =
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;

		srvTable.AddSRVRange<0, 100>(UINT_MAX, flags, 0);
	}

	pipelineDesc.RootSignatureDesc.AddDescriptorTable(srvTable);
	pipelineDesc.RootSignatureDesc.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);
	this->m_pipelineState = this->GetDevice()->CreateGraphicPipeline(pipelineDesc);

	ICommandContext& copyContext = this->GetDevice()->BeginContext();

	this->CreateCube(1, this->m_vertices, this->m_indices, true);
	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(Vertex) * this->m_vertices.size();
		bufferDesc.StrideInBytes = sizeof(Vertex);

		this->m_vertexbuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		copyContext.WriteBuffer<Vertex>(this->m_vertexbuffer, this->m_vertices);
		copyContext.TransitionBarrier(this->m_vertexbuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint16_t) * this->m_indices.size();
		bufferDesc.StrideInBytes = sizeof(uint16_t);

		this->m_indexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<uint16_t>(this->m_indexBuffer, this->m_indices);
		copyContext.TransitionBarrier(this->m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	// TODO: Better deal with asset paths.
	this->m_cubeTexture = 
		this->GetTextureStore()->Load(
			"D:\\Users\\C.DiPaolo\\Development\\GraphicsLearning-Dx12\\Assets\\Checker.dds",
			copyContext,
			BindFlags::ShaderResource);

	this->GetDevice()->Submit(true);
}

void TexturedCubeApp::Update(double elapsedTime)
{
	// Set Matrix data 
	XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	static float i = 0.0f;
	float angle = i;
	i += 10 * elapsedTime;

	const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
	XMMATRIX rotationMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	this->m_cubeWorldTransform = scaleMatrix * rotationMatrix * translationMatrix;
}

void TexturedCubeApp::Render()
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
			this->m_cubeWorldTransform,
			this->m_viewMatrix,
			this->m_porjMatrix,
			drawInfo);


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

		MaterialInfo matInfo = {};
		matInfo.albedoIndex = this->GetDevice()->GetDescritporIndex(this->m_cubeTexture);

		gfxContext.BindGraphics32BitConstants(RootParameters::PushConstant, matInfo);
		gfxContext.BindDynamicConstantBuffer<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
		gfxContext.BindBindlessDescriptorTables(RootParameters::TextureSRV);
		gfxContext.DrawIndexed(this->m_indices.size());
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Transition Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	this->GetDevice()->Submit();
}

void TexturedCubeApp::CreateCube(
	float size,
	std::vector<Vertex>& outVertices,
	std::vector<uint16_t>& outIndices,
	bool rhsCoord)
{
	// A cube has six faces, each one pointing in a different direction.
	const int FaceCount = 6;

	static const XMVECTORF32 faceNormals[FaceCount] =
	{
		{ 0,  0,  1 },
		{ 0,  0, -1 },
		{ 1,  0,  0 },
		{ -1,  0,  0 },
		{ 0,  1,  0 },
		{ 0, -1,  0 },
	};

	static const XMFLOAT3 faceColour[] =
	{
		{ 1.0f,  0.0f,  0.0f },
		{ 0.0f,  1.0f,  0.0f },
		{ 0.0f,  0.0f,  1.0f },
	};

	static const XMVECTORF32 textureCoordinates[4] =
	{
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 },
		{ 0, 0 },
	};

	size /= 2;

	for (int i = 0; i < FaceCount; i++)
	{
		XMVECTOR normal = faceNormals[i];

		// Get two vectors perpendicular both to the face normal and to each other.
		XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

		XMVECTOR side1 = XMVector3Cross(normal, basis);
		XMVECTOR side2 = XMVector3Cross(normal, side1);

		// Six indices (two triangles) per face.
		size_t vbase = outVertices.size();
		outIndices.push_back(static_cast<uint16_t>(vbase + 0));
		outIndices.push_back(static_cast<uint16_t>(vbase + 1));
		outIndices.push_back(static_cast<uint16_t>(vbase + 2));

		outIndices.push_back(static_cast<uint16_t>(vbase + 0));
		outIndices.push_back(static_cast<uint16_t>(vbase + 2));
		outIndices.push_back(static_cast<uint16_t>(vbase + 3));

		// Four vertices per face.
		outVertices.push_back(Vertex((normal - side1 - side2) * size, textureCoordinates[0], faceColour[0]));
		outVertices.push_back(Vertex((normal - side1 + side2) * size, textureCoordinates[1], faceColour[1]));
		outVertices.push_back(Vertex((normal + side1 + side2) * size, textureCoordinates[2], faceColour[2]));
		outVertices.push_back(Vertex((normal + side1 - side2) * size, textureCoordinates[3], faceColour[3]));
	}

	if (rhsCoord)
	{
		ReverseWinding(outIndices, outVertices);
	}
}

void XM_CALLCONV TexturedCubeApp::ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX projection, DrawInfo& drawInfo)
{
	drawInfo.ModelViewProjectionMatrix = model * view * projection;
}