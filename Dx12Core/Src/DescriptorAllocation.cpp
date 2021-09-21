#include "DescriptorAllocation.h"

#include "Dx12DescriptorHeap.h"

using namespace Dx12Core;


DescriptorAllocation::DescriptorAllocation()
	: m_cpuHandle{ 0 }
	, m_gpuHandle{ 0 }
	, m_index(INVALID_INDEX)
	, m_heap(nullptr)
{}

DescriptorAllocation::DescriptorAllocation(
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
	DescriptorIndex index,
	StaticDescriptorHeap* heap)
	: m_cpuHandle{ cpuHandle }
	, m_gpuHandle{ gpuHandle }
	, m_index(index)
	, m_heap(heap)
{
}


DescriptorAllocation::~DescriptorAllocation()
{
	this->Free();
}

DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
	: m_cpuHandle(allocation.m_cpuHandle)
	, m_gpuHandle(allocation.m_gpuHandle)
	, m_index(allocation.m_index)
	, m_heap(allocation.m_heap)
{
	allocation.m_cpuHandle.ptr = 0;
	allocation.m_gpuHandle.ptr = 0;
	allocation.m_index = INVALID_INDEX;
	allocation.m_heap = nullptr;
}

DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other)
{
	// Free this descriptor if it points to anything.
	this->Free();

	this->m_cpuHandle = other.m_cpuHandle;
	this->m_gpuHandle = other.m_gpuHandle;
	this->m_index = other.m_index;
	this->m_heap = other.m_heap;

	other.m_cpuHandle.ptr = 0;
	other.m_gpuHandle.ptr = 0;
	other.m_index = INVALID_INDEX;
	other.m_heap = nullptr;

	return *this;
}

void DescriptorAllocation::Free()
{
	if (!this->IsNull() && this->m_heap)
	{
		this->m_heap->Release(std::move(*this));

		this->m_cpuHandle.ptr = 0;
		this->m_gpuHandle.ptr = 0;
		this->m_index = INVALID_INDEX;
		this->m_heap = nullptr;
	}
}