#include "GfxContext.h"

Dx12Core::GfxContext::GfxContext(RefCountPtr<ID3D12Device2> device)
	: m_allocatorPool(device, D3D12_COMMAND_LIST_TYPE_DIRECT)
{
	this->m_currentAllocator = this->m_allocatorPool.RequestAllocator(0);

	ThrowIfFailed(
		device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			this->m_currentAllocator,
			nullptr, IID_PPV_ARGS(&this->m_internalCommandList)));
}

void Dx12Core::GfxContext::Reset(uint64_t completedFenceValue)
{
	assert(this->m_currentAllocator == nullptr);
	this->m_currentAllocator = this->m_allocatorPool.RequestAllocator(completedFenceValue);
	
	this->m_internalCommandList->Reset(this->m_currentAllocator, nullptr);
}

void Dx12Core::GfxContext::Executed(uint64_t fenceValue)
{
	this->m_allocatorPool.DiscardAllocator(fenceValue, this->m_currentAllocator);
	this->m_currentAllocator = nullptr;
}

void Dx12Core::GfxContext::Close()
{
	this->m_internalCommandList->Close();
}

void Dx12Core::GfxContext::TransitionBarrier(ResourceId texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
}

void Dx12Core::GfxContext::ClearRenderTarget(ResourceId rexture, Color const& color)
{
}
