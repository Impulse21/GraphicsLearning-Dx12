
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

#include "Shaders/TriangleVS_compiled.h"
#include "Shaders/TrianglePS_compiled.h"

#include <DirectXMath.h>

using namespace Dx12Core;
using namespace DirectX;

class TriangleApp : public ApplicationDx12Base
{
public:
	TriangleApp() = default;

protected:
	void LoadContent() override;
	void Update(double elapsedTime) override {};
	void Render() override;

private:
	GraphicsPipelineHandle m_pipelineState;
	BufferHandle m_vertexbuffer;
	BufferHandle m_indexBuffer;
};


CREATE_APPLICATION(TriangleApp)


struct Vertex
{
	XMFLOAT2 Positon;
	XMFLOAT4 Colour;
};

static std::vector<Vertex> sVertices =
{
	{ XMFLOAT2(-0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), },
	{ XMFLOAT2(0.0f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),},
	{ XMFLOAT2(0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),},
};

static std::vector<uint32_t> sIndices =
{
	0,
	1,
	2
};

void TriangleApp::LoadContent()
{
	ShaderDesc d = {};
	d.shaderType = ShaderType::Vertex;

	ShaderHandle vs = this->GetDevice()->CreateShader(d, gTriangleVS, sizeof(gTriangleVS));

	d.shaderType = ShaderType::Pixel;
	ShaderHandle ps = this->GetDevice()->CreateShader(d, gTrianglePS, sizeof(gTrianglePS));

	GraphicsPipelineDesc pipelineDesc = {};
	pipelineDesc.InputLayout =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(this->GetDevice()->GetCurrentSwapChainDesc().Format);
	pipelineDesc.RootSignatureDesc.AllowInputLayout();
	this->m_pipelineState = this->GetDevice()->CreateGraphicPipeline(pipelineDesc);

	ICommandContext& copyContext = this->GetDevice()->BeginContext();

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(Vertex) * sVertices.size();
		bufferDesc.StrideInBytes = sizeof(Vertex);

		this->m_vertexbuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		copyContext.WriteBuffer<Vertex>(this->m_vertexbuffer, sVertices);
		copyContext.TransitionBarrier(this->m_vertexbuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint32_t) * sIndices.size();
		bufferDesc.StrideInBytes = sizeof(uint32_t);

		this->m_indexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<uint32_t>(this->m_indexBuffer, sIndices);
		copyContext.TransitionBarrier(this->m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	this->GetDevice()->Submit(true);
}

void TriangleApp::Render()
{
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Clearing Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 0.0f, 1.0f });
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

		gfxContext.SetGraphicsState(s);

		gfxContext.DrawIndexed(3);
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Transition Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	this->GetDevice()->Submit();
}
