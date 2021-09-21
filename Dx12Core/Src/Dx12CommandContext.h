#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"
#include "Dx12Resources.h"

#include "Dx12Core/Dx12CommandAllocatorPool.h"

#include <memory>

namespace Dx12Core
{
	struct ReferencedResources
	{
		std::vector<RefCountPtr<IResource>> Resources;
		std::vector<RefCountPtr<ID3D12Resource>> NativeResources;
	};

	class Dx12CommandContext : public RefCounter<ICommandContext>
	{
	public:
		explicit Dx12CommandContext(RefCountPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::wstring const& debugName = L"");

		void Reset(uint64_t completedFenceValue);
		void Close();
		std::shared_ptr<ReferencedResources> Executed(uint64_t fenceValue);

		ID3D12GraphicsCommandList* GetInternal() { return this->m_internalList; }

		// -- Wrapped Commands ---
	public:
		void TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) override;
		void ClearTextureFloat(ITexture* texture, Color const& clearColour) override;

		virtual void BeginMarker(std::string name);
		virtual void EndMarker();

	private:
		RefCountPtr<ID3D12GraphicsCommandList> m_internalList;
		ID3D12CommandAllocator* m_allocator;

		CommandAllocatorPool m_allocatorPool;

		std::shared_ptr<ReferencedResources> m_trackedResources;
	};
}

