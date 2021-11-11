#pragma once

#include <memory>
#include <assert.h>
#include <array>

#include "Dx12Core.h"
#include "Dx12Common.h"
#include "ResouceIdOwner.h"

// -- depericated ---
#include "Dx12DescriptorHeap.h"
#include "DescriptorHeap.h"

#include "Dx12CommandContext.h"

#include "Dx12Core/Dx12Queue.h"

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

	enum DescritporHeapType
	{
		Srv_Cbv_Uav = 0,
		Sampler,
		Rtv,
		Dsv,
		NumTypes
	};

	class StaleResourceWrapper
	{
	public:
		class IStaleResource;

		template<typename ResourceType>
		static StaleResourceWrapper Create(ResourceType&& type)
		{
			class SpecificStaleResource : public IStaleResource
			{
			public:
				SpecificStaleResource(ResourceType&& resource)
					: m_resource(std::move(resource))
				{}

				SpecificStaleResource(const SpecificStaleResource&) = delete;
				SpecificStaleResource(SpecificStaleResource&&) = delete;
				SpecificStaleResource& operator = (const SpecificStaleResource&) = delete;
				SpecificStaleResource& operator = (SpecificStaleResource&&) = delete;

			private:
				ResourceType m_resource;
			};

			return StaleResourceWrapper(std::make_shared<SpecificStaleResource>(std::move(type)));
		}

		StaleResourceWrapper(std::shared_ptr<IStaleResource> staleResource)
			: m_staleResource(staleResource)
		{}

		StaleResourceWrapper(StaleResourceWrapper&& other) noexcept 
			: m_staleResource(std::move(other.m_staleResource))
		{
			other.m_staleResource = nullptr;
		}

		StaleResourceWrapper(const StaleResourceWrapper& other) noexcept 
			: m_staleResource(other.m_staleResource)
		{
		}

		// clang-format off
		StaleResourceWrapper& operator = (const StaleResourceWrapper&) = delete;
		StaleResourceWrapper& operator = (StaleResourceWrapper&&) = delete;
		// clang-format on

	private:
		class IStaleResource
		{
		public:
			virtual ~IStaleResource() = default;
		};

	private:
		std::shared_ptr<IStaleResource> m_staleResource;
	};

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

		template<typename ResourceType>
		void SafeReleaseObject(ResourceType&& resource)
		{
			auto staleWrapper = StaleResourceWrapper::Create(std::move(resource));

			// I will need to update this to support other queues but for now, not a big deal as I only support graphics queue
			auto fenceValue = this->GetGfxQueue()->GetLastCompletedFence() + 1L;
			this->m_safeReleaseQueue.emplace_back(std::make_tuple(fenceValue, staleWrapper));
		};

	public:
		Dx12Queue* GetGfxQueue() { return this->m_queues[static_cast<size_t>(CommandQueue::Graphics)].get(); }

		const Dx12Context& GetDx12Context() const { return this->m_context; }
		ID3D12Device2* GetDx12Device2() const { return this->m_context.Device2.Get(); }

		CpuDescriptorHeap* GetCpuHeap(DescritporHeapType type)
		{
			return this->m_cpuDescriptorHeaps[type].get();
		}
		
		GpuDescriptorHeap* GetGpuResourceHeap()
		{
			return this->m_gpuDescritporHeaps[Srv_Cbv_Uav].get();
		}

		GpuDescriptorHeap* GetGpuSamplerHeap()
		{
			return this->m_gpuDescritporHeaps[Sampler].get();
		}

	private:
		RootSignatureHandle CreateRootSignature(RootSignatureDesc& desc);
		RootSignatureHandle CreateRootSignature(
			D3D12_ROOT_SIGNATURE_FLAGS flags,
			ShaderParameterLayout* shaderParameter,
			BindlessShaderParameterLayout* bindlessLayout);
		RefCountPtr<ID3D12RootSignature> CreateD3DRootSignature(D3D12_ROOT_SIGNATURE_DESC1&& rootSigDesc);

		void CollectShaderParameters(const void* binary, size_t binarySize);

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

		std::array<std::unique_ptr<CpuDescriptorHeap>, DescritporHeapType::NumTypes> m_cpuDescriptorHeaps;
		std::array<std::unique_ptr<GpuDescriptorHeap>, 2> m_gpuDescritporHeaps;

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

		std::deque<std::tuple<uint64_t, StaleResourceWrapper>> m_safeReleaseQueue;
	};
}

