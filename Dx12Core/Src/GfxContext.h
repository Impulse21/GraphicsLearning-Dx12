#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"
#include "Dx12Core/Dx12CommandAllocatorPool.h"

namespace Dx12Core
{
	class GfxContext : public ICommandContext
	{
	public:
		GfxContext(RefCountPtr<ID3D12Device2> device);

		void Reset(uint64_t completedFenceValue);
		void Executed(uint64_t fenceValue);

		void Begin() {}
		void Close();

		void TransitionBarrier(ResourceId texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

		void ClearRenderTarget(ResourceId rexture, Color const& color);

		void BeginMarker(std::string name);
		void EndMarker();

	private:
		RefCountPtr<ID3D12GraphicsCommandList> m_internalCommandList;

		ID3D12CommandAllocator* m_currentAllocator;
		CommandAllocatorPool m_allocatorPool;
	};
}