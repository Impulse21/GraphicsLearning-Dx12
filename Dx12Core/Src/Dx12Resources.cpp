#include "Dx12Core/Dx12Resources.h"

#include "Dx12Core/Dx12DescriptorHeap.h"

using namespace Dx12Core;

Texture::Texture(
	Dx12Context const& context,
	DeviceResources const& resources,
	TextureDesc desc,
	RefCountPtr<ID3D12Resource> nativeResource)
	: m_context(context)
	, m_deviceResources(resources)
	, m_desc(std::move(desc))
	, m_resource(nativeResource)
{

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
		this->m_context.Device2->CreateRenderTargetView(this->m_resource, nullptr, handle);
	}
}
