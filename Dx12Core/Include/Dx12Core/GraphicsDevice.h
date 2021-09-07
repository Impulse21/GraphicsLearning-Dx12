#pragma once

#include <memory>

#include <assert.h>
#include "Dx12Common.h"
#include "Dx12Resources.h"

namespace Dx12Core
{
	enum class CommandQueue : uint8_t
	{
		Graphics = 0,
		Compute,
		Copy,

		Count
	};

	class Dx12Queue;
	class GraphicsDevice : public RefCounter<IGraphicsDevice>
	{
	public:
		GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context context);
		~GraphicsDevice() override;

		void InitializeSwapcChain(SwapChainDesc const& swapChainDesc) override;

		void BeginFrame();
		void Present();

		void WaitForIdle() const;

	public:
		TextureHandle CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native);

	public:
		const GraphicsDeviceDesc& GetDesc() const override { return this->m_desc; }
		const SwapChainDesc& GetCurrentSwapChainDesc() const override { return this->m_swapChainDesc; }

		uint32_t GetCurrentBackBuffer() { return this->GetBackBuffer(this->GetCurrentBackBufferIndex()); }
		uint32_t GetBackBuffer(uint32_t index) { return 1; };
		uint32_t GetCurrentBackBufferIndex() { assert(this->m_swapChain); return this->m_swapChain->GetCurrentBackBufferIndex(); }

	private:
		void InitializeRenderTargets();

	private:
		Dx12Queue* GetGfxQueue() { return this->m_queues[static_cast<size_t>(CommandQueue::Graphics)].get(); }

	private:
		const Dx12Context m_context;
		const GraphicsDeviceDesc m_desc;
		DeviceResources m_deviceResources;

		SwapChainDesc m_swapChainDesc;

		RefCountPtr<IDXGISwapChain4> m_swapChain;

		std::array<std::unique_ptr<Dx12Queue>, static_cast<size_t>(CommandQueue::Count)> m_queues;
		std::vector<TextureHandle> m_swapChainTextures;
		std::vector<uint64_t> m_frameFence;
	};
}

