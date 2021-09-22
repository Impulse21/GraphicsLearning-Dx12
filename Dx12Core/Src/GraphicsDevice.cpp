#include "Dx12Core/GraphicsDevice.h"
#include "Dx12Core/Dx12Queue.h"
#include "Dx12DescriptorHeap.h"

using namespace Dx12Core;

Dx12Core::GraphicsDevice::GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context& context)
	: m_context(context)
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

	this->m_renderTargetViewHeap = 
		std::make_unique<StaticDescriptorHeap>(
			this->m_context,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			desc.RenderTargetViewHeapSize);
}

Dx12Core::GraphicsDevice::~GraphicsDevice()
{
	this->WaitForIdle();

	this->m_swapChainTextures.clear();

	this->m_context.Device5.Reset();
	this->m_context.Device2.Reset();
	this->m_context.Device.Reset();
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

	this->m_frames.resize(this->m_swapChainDesc.NumBuffers);
	this->InitializeRenderTargets();
}

ICommandContext& Dx12Core::GraphicsDevice::BeginContext()
{
	std::scoped_lock(this->m_commandListMutex);
	ContextId contextId = this->m_activeContext++;

	assert(contextId < MAX_COMMAND_CONTEXT);
	if (!this->m_commandContexts[contextId])
	{
		this->m_commandContexts[contextId] = 
			std::make_unique<Dx12CommandContext>(
				this->m_context.Device2,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				std::wstring(L"Command List: ") + std::to_wstring(contextId));
	}
	else
	{
		this->m_commandContexts[contextId]->Reset(this->GetGfxQueue()->GetLastCompletedFence());
	}

	return *this->m_commandContexts[contextId];
}

uint64_t Dx12Core::GraphicsDevice::Submit()
{
	std::scoped_lock(this->m_commandListMutex);
	ContextId cmdLast = this->m_activeContext;
	this->m_activeContext = 0;

	this->m_commandListsToExecute.resize(cmdLast);
	for (ContextId i = 0; i < cmdLast; i++)
	{
		Dx12CommandContext& context = *this->m_commandContexts[i];
		context.Close();
		this->m_commandListsToExecute[i] = context.GetInternal();
	}

	uint64_t fence = this->GetGfxQueue()->ExecuteCommandLists(this->m_commandListsToExecute);

	for (ContextId i = 0; i < cmdLast; i++)
	{
		auto trackedResources = this->m_commandContexts[i]->Executed(fence);
		this->GetCurrentFrame().ReferencedResources.push_back(trackedResources);
	}

	return fence;
}

void Dx12Core::GraphicsDevice::BeginFrame()
{
	uint32_t bufferIndex = this->GetCurrentBackBufferIndex();

	Frame& frame = this->GetCurrentFrame();
	this->GetGfxQueue()->WaitForFence(frame.FrameFence);

	frame.ReferencedResources.clear();
}

void Dx12Core::GraphicsDevice::Present()
{
	this->m_swapChain->Present(0, 0);

	this->GetCurrentFrame().FrameFence = this->GetGfxQueue()->IncrementFence();

	this->m_frame = (this->m_frame + 1) % this->m_swapChainDesc.NumBuffers;
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
	// Use unique ptr here to ensure safety until we are able to pass this over to the texture handle
	std::unique_ptr<Texture> internal = std::make_unique<Texture>(this, desc);
	internal->D3DResource = native;

	if ((internal->GetDesc().Bindings & BindFlags::RenderTarget) == BindFlags::RenderTarget)
	{
		internal->Rtv = this->m_renderTargetViewHeap->AllocateDescriptor();
		this->m_context.Device2->CreateRenderTargetView(internal->D3DResource, nullptr, internal->Rtv.GetCpuHandle());
	}

	std::wstring debugName(internal->GetDesc().DebugName.begin(), internal->GetDesc().DebugName.end());
	internal->D3DResource->SetName(debugName.c_str());

	return TextureHandle::Create(internal.release());
}

void Dx12Core::GraphicsDevice::InitializeRenderTargets()
{
	this->m_swapChainTextures.resize(this->m_swapChainDesc.NumBuffers);
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
	}
}
