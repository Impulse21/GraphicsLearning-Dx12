#pragma once

#include <assert.h>
#include "Dx12Common.h"
#include "Dx12Resources.h"

namespace Dx12Core
{
	class GraphicsDevice : public RefCounter<IGraphicsDevice>
	{
	public:
		GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context context);
		~GraphicsDevice() override = default;

		void InitializeSwapcChain(SwapChainDesc swapChainDesc);

		virtual void BeginFrame() = 0;
		virtual void Present() = 0;

	public:
		const GraphicsDeviceDesc& GetDesc() const override { return this->m_desc; }
		const SwapChainDesc& GetCurrentSwapChainDesc() const override { return this->m_swapChainDesc; }

		uint32_t GetCurrentBackBuffer() { this->GetBackBuffer(this->GetCurrentBackBufferIndex()); }
		uint32_t GetBackBuffer(uint32_t index) { return 1; };
		uint32_t GetCurrentBackBufferIndex() { assert(this->m_swapChain); this->m_swapChain->GetCurrentBackBufferIndex(); }

	private:
		void InitializeRenderTargets();

	private:
		const Dx12Context m_context;
		const GraphicsDeviceDesc m_desc;
		SwapChainDesc m_swapChainDesc;

		RefCountPtr<IDXGISwapChain4> m_swapChain;

	};
}

