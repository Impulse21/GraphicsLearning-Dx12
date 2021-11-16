#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"
#include "Dx12Resources.h"

#include "Dx12Core/Dx12CommandAllocatorPool.h"
#include "Dx12UploadBuffer.h"

#include <memory>

namespace Dx12Core
{
	struct ReferencedResources
	{
		std::vector<RefCountPtr<IResource>> Resources;
		std::vector<RefCountPtr<ID3D12Resource>> NativeResources;
	};

	class StaticDescriptorHeap;
	class GpuDescriptorHeap;
	class Dx12CommandContext : public RefCounter<ICommandContext>
	{
	public:
		explicit Dx12CommandContext(
			RefCountPtr<ID3D12Device2> device,
			D3D12_COMMAND_LIST_TYPE type,
			std::wstring const& debugName = L"");

		void Reset(uint64_t completedFenceValue);
		void Close();
		std::shared_ptr<ReferencedResources> Executed(uint64_t fenceValue);

		ID3D12GraphicsCommandList* GetInternal() { return this->m_internalList; }

		void BindHeaps(std::array<GpuDescriptorHeap*, 2> const& shaderHeaps);

		// -- Wrapped Commands ---
	public:

		void TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) override;
		void TransitionBarrier(IBuffer* buffer, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) override;
		void ClearTextureFloat(ITexture* texture, Color const& clearColour) override;
		void ClearDepthStencilTexture(ITexture* depthStencil, bool clearDepth, float depth, bool clearStencil, uint8_t stencil) override;

		void SetGraphicsState(GraphicsState& state) override;
		void BindScissorRects(std::vector<Rect> const& rects) override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0) override;
		void DrawIndexed(
			uint32_t indexCount,
			uint32_t instanceCount = 1,
			uint32_t startIndex = 0,
			int32_t baseVertex = 0,
			uint32_t startInstance = 0) override;

		void WriteBuffer(IBuffer* buffer, const void* data, size_t dataSize, uint64_t destOffsetBytes = 0) override;
		void WriteTexture(
			ITexture* texture,
			uint32_t firstSubResource,
			size_t numSubResources,
			D3D12_SUBRESOURCE_DATA* subresourceData) override;

		void WriteTexture(
			ITexture* texture,
			uint32_t arraySlize,
			uint32_t mipLevel,
			const void* data,
			size_t rowPitch,
			size_t depthPitch) override;


		void BindGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants) override;
		void BindDynamicConstantBuffer(size_t rootParameterIndex, size_t sizeInBytes, const void* bufferData) override;
		void BindDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
		void BindDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
		void BindDynamicStructuredBuffer(uint32_t rootParameterIndex, size_t numElements, size_t elementSize, const void* bufferData);
		void BindStructuredBuffer(size_t rootParameterIndex, IBuffer* buffer) override;
		void BindBindlessDescriptorTables(size_t rootParamterIndex) override;

		virtual ScopedMarker BeginScropedMarker(std::string name) override;
		virtual void BeginMarker(std::string name);
		virtual void EndMarker();

	private:
		RefCountPtr<ID3D12Device2> m_device;
		RefCountPtr<ID3D12GraphicsCommandList> m_internalList;
		ID3D12CommandAllocator* m_allocator;

		std::unique_ptr<UploadBuffer> m_uploadBuffer;

		CommandAllocatorPool m_allocatorPool;

		D3D12_RECT m_dx12Scissor[16] = {};
		UINT m_numScissor;

		std::shared_ptr<ReferencedResources> m_trackedResources;
	};
}

