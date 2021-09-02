#include "Dx12Core/GraphicsDevice.h"

using namespace Dx12Core;

Dx12Core::GraphicsDevice::GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context context)
	: m_context(std::move(context))
	, m_desc(desc)
{

}

void Dx12Core::GraphicsDevice::InitializeSwapcChain(SwapChainDesc swapChainDesc)
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
			this->m_queues[static_cast<int>(CommandQueue::Graphics)]->GetNative(),
			swapChainDesc.WindowHandle,
			&dx12Desc,
			nullptr,
			nullptr,
			&tmpSwapChain));

	ThrowIfFailed(
		tmpSwapChain->QueryInterface(&this->m_swapChain));

}
