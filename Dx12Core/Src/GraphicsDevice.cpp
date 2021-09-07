#include "Dx12Core/GraphicsDevice.h"
#include "Dx12Core/Dx12Queue.h"
#include "Dx12Core/Dx12DescriptorHeap.h"

using namespace Dx12Core;

Dx12Core::GraphicsDevice::GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context context)
	: m_context(std::move(context))
	, m_desc(std::move(desc))
{
	this->m_queues[static_cast<size_t>(CommandQueue::Graphics)] 
		= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_DIRECT);

	if (this->m_desc.EnableComputeQueue)
	{
		this->m_queues[static_cast<size_t>(CommandQueue::Compute)] 
			= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}

	if (this->m_desc.EnableCopyQueue)
	{
		this->m_queues[static_cast<size_t>(CommandQueue::Copy)] 
			= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_COPY);
	}

	this->m_deviceResources.RenderTargetViewHeap = 
		std::make_unique<StaticDescriptorHeap>(
			this->m_context,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			desc.RenderTargetViewHeapSize);
}

Dx12Core::GraphicsDevice::~GraphicsDevice()
{
	this->WaitForIdle();
	this->m_swapChainTextures.clear();
}

void Dx12Core::GraphicsDevice::InitializeSwapcChain(SwapChainDesc const& swapChainDesc)
{
	assert(swapChainDesc.WindowHandle);
	if (!swapChainDesc.WindowHandle)
	{
		LOG_CORE_ERROR("Invalid window handle");
		throw std::runtime_error("Invalid Window Error");
	}

	DXGI_SWAP_CHAIN_DESC1 dx12Desc = {};
	dx12Desc.Width = swapChainDesc.Width;
	dx12Desc.Height = swapChainDesc.Height;
	dx12Desc.Format = swapChainDesc.Format;
	dx12Desc.SampleDesc.Count = 1;
	dx12Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dx12Desc.BufferCount = swapChainDesc.NumBuffers;
	dx12Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dx12Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;


	RefCountPtr<IDXGISwapChain1> tmpSwapChain;
	ThrowIfFailed(
		this->m_context.Factory->CreateSwapChainForHwnd(
			this->m_queues[D3D12_COMMAND_LIST_TYPE_DIRECT]->GetNative(),
			swapChainDesc.WindowHandle,
			&dx12Desc,
			nullptr,
			nullptr,
			&tmpSwapChain));

	ThrowIfFailed(
		tmpSwapChain->QueryInterface(&this->m_swapChain));

	this->m_swapChainDesc = swapChainDesc;
	this->InitializeRenderTargets();
}

void Dx12Core::GraphicsDevice::BeginFrame()
{
	uint32_t bufferIndex = this->GetCurrentBackBufferIndex();

	this->GetGfxQueue()->WaitForFence(this->m_frameFence[bufferIndex]);

}

void Dx12Core::GraphicsDevice::Present()
{
	this->m_swapChain->Present(0, 0);

	this->m_frameFence[this->GetCurrentBackBufferIndex()] = this->GetGfxQueue()->IncrementFence();
}

void Dx12Core::GraphicsDevice::WaitForIdle() const
{
	for (auto& queue : this->m_queues)
	{
		if (queue)
		{
			queue->WaitForIdle();
		}
	}
}

TextureHandle Dx12Core::GraphicsDevice::CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native)
{
	std::unique_ptr<Texture> texture = std::make_unique<Texture>(this->m_context, this->m_deviceResources, desc, native);

	texture->CreateViews();

	return TextureHandle::Create(texture.release());
}

void Dx12Core::GraphicsDevice::InitializeRenderTargets()
{
	this->m_swapChainTextures.resize(this->m_swapChainDesc.NumBuffers);
	this->m_frameFence.resize(this->m_swapChainDesc.NumBuffers);
	for (UINT i = 0; i < this->m_swapChainDesc.NumBuffers; i++)
	{
		RefCountPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(
			this->m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		auto textureDesc = TextureDesc();
		textureDesc.Dimension = TextureDimension::Texture2D;
		textureDesc.Format = this->m_swapChainDesc.Format;
		textureDesc.Width = this->m_swapChainDesc.Width;
		textureDesc.Height = this->m_swapChainDesc.Height;
		textureDesc.Bindings = BindFlags::RenderTarget;

		this->m_swapChainTextures[i] = this->CreateTextureFromNative(textureDesc, backBuffer);
		this->m_frameFence[i] = 1;
	}
}
