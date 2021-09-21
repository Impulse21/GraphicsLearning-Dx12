#include "Dx12DescriptorHeap.h"

using namespace Dx12Core;

StaticDescriptorHeap::StaticDescriptorHeap(
	Dx12Context const& context,
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	uint32_t numDescriptorsInHeap,
	bool isShaderVisible)
	: m_context(context)
	, m_heapType(heapType)
	, m_isShaderVisible(isShaderVisible)
{

	this->m_desc = {};
	this->m_desc.Type = heapType;
	this->m_desc.NumDescriptors = numDescriptorsInHeap;
	this->m_desc.Flags = this->m_isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(
		this->m_context.Device2->CreateDescriptorHeap(&this->m_desc, IID_PPV_ARGS(&this->m_heap)));

	this->m_descriptorHandleIncrementSize = this->m_context.Device2->GetDescriptorHandleIncrementSize(heapType);

	this->m_cpuStartHandle = this->m_heap->GetCPUDescriptorHandleForHeapStart();
	this->m_gpuStartHandle = this->m_isShaderVisible ? this->m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE();

	this->m_descriptorIndexPool = { numDescriptorsInHeap };
}

DescriptorAllocation Dx12Core::StaticDescriptorHeap::AllocateDescriptor()
{
	auto descriptorIndex = this->m_descriptorIndexPool.Allocate();
	return DescriptorAllocation(
		this->GetCpuHandle(descriptorIndex),
		this->GetGpuHandle(descriptorIndex),
		descriptorIndex,
		this);
}

void Dx12Core::StaticDescriptorHeap::Release(DescriptorAllocation&& allocation)
{
	this->m_descriptorIndexPool.Release(allocation.GetIndex());
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12Core::StaticDescriptorHeap::GetCpuHandle(DescriptorIndex index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(this->m_cpuStartHandle, index, this->m_descriptorHandleIncrementSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE Dx12Core::StaticDescriptorHeap::GetGpuHandle(DescriptorIndex index)
{
	if (!this->IsShaderVisible())
	{
		return this->m_gpuStartHandle;
	}

	return CD3DX12_GPU_DESCRIPTOR_HANDLE(this->m_gpuStartHandle, index, this->m_descriptorHandleIncrementSize);
}
