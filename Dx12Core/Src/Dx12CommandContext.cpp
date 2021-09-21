#include "Dx12CommandContext.h"

#include <pix.h>

using namespace Dx12Core;
Dx12CommandContext::Dx12CommandContext(RefCountPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::wstring const& debugName)
	: m_allocatorPool(device, type)
{
	this->m_allocator = this->m_allocatorPool.RequestAllocator(0);

	ThrowIfFailed(
		device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			this->m_allocator,
			nullptr,
			IID_PPV_ARGS(&this->m_internalList)));

	this->m_trackedResources = std::make_shared<ReferencedResources>();

	this->m_internalList->SetName(debugName.c_str());
}

void Dx12Core::Dx12CommandContext::Reset(uint64_t completedFenceValue)
{
	assert(this->m_allocator == nullptr);

	this->m_allocator = this->m_allocatorPool.RequestAllocator(completedFenceValue);

	this->m_internalList->Reset(this->m_allocator, nullptr);

	this->m_trackedResources = std::make_shared<ReferencedResources>();
}

void Dx12Core::Dx12CommandContext::Close()
{
	this->m_internalList->Close();
}

std::shared_ptr<ReferencedResources> Dx12Core::Dx12CommandContext::Executed(uint64_t fenceValue)
{
	this->m_allocatorPool.DiscardAllocator(fenceValue, this->m_allocator);
	this->m_allocator = nullptr;
	auto retVal = this->m_trackedResources;
	this->m_trackedResources.reset();
	return retVal;
}

void Dx12Core::Dx12CommandContext::TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	Texture* internal = SafeCast<Texture*>(texture);
	assert(internal);
	this->m_trackedResources->Resources.push_back(internal);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		internal->D3DResource,
		beforeState, afterState);

	this->m_internalList->ResourceBarrier(1, &barrier);
}

void Dx12Core::Dx12CommandContext::ClearTextureFloat(ITexture* texture, Color const& clearColour)
{
	Texture* internal = SafeCast<Texture*>(texture);
	assert(internal);

	this->m_trackedResources->Resources.push_back(internal);
	if ((internal->GetDesc().Bindings & BindFlags::RenderTarget) == BindFlags::RenderTarget)
	{
		this->m_internalList->ClearRenderTargetView(internal->Rtv.GetCpuHandle(), &clearColour.R, 0, nullptr);
	}
}

void Dx12Core::Dx12CommandContext::BeginMarker(std::string name)
{
	PIXBeginEvent(this->m_internalList, 0, std::wstring(name.begin(), name.end()).c_str());
}

void Dx12Core::Dx12CommandContext::EndMarker()
{
	PIXEndEvent(this->m_internalList);
}
