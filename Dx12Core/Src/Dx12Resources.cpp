#include "Dx12Core/Dx12Resources.h"

#include "Dx12Core/Dx12DescriptorHeap.h"
#include "Dx12Core/GraphicsDevice.h"

using namespace Dx12Core;

Texture::Texture(
	Dx12Context const& context,
	DeviceResources const& resources,
	TextureDesc desc,
	RefCountPtr<ID3D12Resource> nativeResource)
	: m_context(context)
	, m_deviceResources(resources)
	, m_desc(std::move(desc))
{
	this->m_d3dResource = nativeResource;
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12Core::Texture::GetRtv()
{
	assert(this->m_rtv != INVALID_DESCRIPTOR_INDEX);

	if (this->m_rtv == INVALID_DESCRIPTOR_INDEX)
	{
		LOG_CORE_ERROR("Invalid Descriptor Index");
		throw std::runtime_error("invalid view");
	}

	return this->m_deviceResources.RenderTargetViewHeap->GetCpuHandle(this->m_rtv);
}

void Dx12Core::Texture::CreateViews()
{
	if ((this->m_desc.Bindings & BindFlags::RenderTarget) == BindFlags::RenderTarget)
	{
		this->m_rtv = this->m_deviceResources.RenderTargetViewHeap->AllocateDescriptor();
		auto handle = this->m_deviceResources.RenderTargetViewHeap->GetCpuHandle(this->m_rtv);
		this->m_context.Device2->CreateRenderTargetView(this->m_d3dResource, nullptr, handle);
	}
}

Dx12Core::CommandContext::CommandContext(GraphicsDevice* device)
	: m_device(device)
	, m_allocatorPool(device->GetDx12Context().Device2, D3D12_COMMAND_LIST_TYPE_DIRECT)
	, m_activeAllocator(nullptr)
{
}

void Dx12Core::CommandContext::Begin()
{
	if (!this->m_activeAllocator)
	{
		this->m_activeAllocator = 
			this->m_allocatorPool.RequestAllocator(this->m_device->GetGfxQueue()->GetLastCompletedFence());
	}

	ThrowIfFailed(
		this->m_internalCommandList->Reset(this->m_activeAllocator, nullptr));
}

void Dx12Core::CommandContext::Close()
{
	this->m_internalCommandList->Close();
}

void Dx12Core::CommandContext::TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	Texture* internalTexture = SafeCast<Texture*>(texture);
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		internalTexture->GetNative(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	this->m_internalCommandList->ResourceBarrier(1, &barrier);
}

void Dx12Core::CommandContext::ClearRenderTarget(ITexture* texture, Color const& color)
{
	Texture* internalTexture = SafeCast<Texture*>(texture);
	this->m_internalCommandList->ClearRenderTargetView(internalTexture->GetRtv(), &color.R, 0, nullptr);
}

void Dx12Core::CommandContext::BeginMarker(std::string name)
{
	PIXBeginEvent(this->m_activeCommandList->commandList, 0, name.c_str());
}

void Dx12Core::CommandContext::EndMarker()
{
	PIXEndEvent(this->m_activeCommandList->commandList);
}

void Dx12Core::CommandContext::Executed(uint64_t fenceValue)
{
	this->m_allocatorPool.DiscardAllocator(fenceValue, this->m_activeAllocator);
	this->m_activeAllocator = nullptr;
}
