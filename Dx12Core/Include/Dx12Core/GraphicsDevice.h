#pragma once

#include <memory>
#include <assert.h>
#include <array>

#include "Dx12Core.h"
#include "Dx12Common.h"
#include "ResouceIdOwner.h"
#include "Dx12DescriptorHeap.h"

#include "Dx12CommandContext.h"

#define MAX_COMMAND_CONTEXT 30 // According to Nvida, this is good.

namespace Dx12Core
{
	typedef uint8_t ContextId;
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
		GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context& context);
		~GraphicsDevice() override;

		void InitializeSwapcChain(SwapChainDesc const& swapChainDesc) override;

		void BeginFrame();
		void Present();

		ICommandContext& BeginContext() override;
		uint64_t Submit(bool waitForCompletion = false) override;

		void WaitForIdle() const;

	public:
		DescriptorIndex GetDescritporIndex(ITexture* texture) const override;
		DescriptorIndex GetDescritporIndex(IBuffer* buffer) const override;

		TextureHandle CreateTexture(TextureDesc desc) override;
		TextureHandle CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native) override;
		BufferHandle CreateBuffer(BufferDesc desc) override;

		ShaderHandle CreateShader(ShaderDesc const& desc, const void* binary, size_t binarySize) override;
		GraphicsPipelineHandle CreateGraphicPipeline(GraphicsPipelineDesc desc) override;

	public:
		const GraphicsDeviceDesc& GetDesc() const override { return this->m_desc; }
		const SwapChainDesc& GetCurrentSwapChainDesc() const override { return this->m_swapChainDesc; }

		TextureHandle GetCurrentBackBuffer() override { return this->GetBackBuffer(this->GetCurrentBackBufferIndex()); }
		TextureHandle GetBackBuffer(uint32_t index) override { return this->m_swapChainTextures[index]; };
		uint32_t GetCurrentBackBufferIndex() { assert(this->m_swapChain); return this->m_swapChain->GetCurrentBackBufferIndex(); }

	public:
		Dx12Queue* GetGfxQueue() { return this->m_queues[static_cast<size_t>(CommandQueue::Graphics)].get(); }

		const Dx12Context& GetDx12Context() const { return this->m_context; }

	private:
		RootSignatureHandle CreateRootSignature(RootSignatureDesc& desc);
		RootSignatureHandle CreateRootSignature(
			D3D12_ROOT_SIGNATURE_FLAGS flags,
			ShaderParameterLayout* shaderParameter,
			BindlessShaderParameterLayout* bindlessLayout);
		RefCountPtr<ID3D12RootSignature> CreateD3DRootSignature(D3D12_ROOT_SIGNATURE_DESC1&& rootSigDesc);

	private:
		struct Frame
		{
			uint64_t FrameFence = 1;
			std::vector<std::shared_ptr<ReferencedResources>> ReferencedResources;
		};

	private:
		void InitializeRenderTargets();
		Frame& GetCurrentFrame() { return this->m_frames[this->m_frame]; }

	private:
		Dx12Context m_context;
		const GraphicsDeviceDesc m_desc;

		std::unique_ptr<StaticDescriptorHeap> m_renderTargetViewHeap;
		std::unique_ptr<StaticDescriptorHeap> m_depthStencilViewHeap;
		std::unique_ptr<StaticDescriptorHeap> m_shaderResourceViewHeap;
		std::unique_ptr<StaticDescriptorHeap> m_samplerHeap;

		SwapChainDesc m_swapChainDesc;

		RefCountPtr<IDXGISwapChain4> m_swapChain;

		std::vector<Frame> m_frames;
		std::array<std::unique_ptr<Dx12CommandContext>, MAX_COMMAND_CONTEXT> m_commandContexts;
		uint8_t m_activeContext = 0;

		uint8_t m_frame = 0;

		std::array<std::unique_ptr<Dx12Queue>, static_cast<size_t>(CommandQueue::Count)> m_queues;
		std::vector<TextureHandle> m_swapChainTextures;

		std::vector<ID3D12CommandList*> m_commandListsToExecute; // used to avoid re-allocations;

		std::mutex m_commandListMutex;
	};
}

