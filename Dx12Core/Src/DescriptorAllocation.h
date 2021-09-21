#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"

#define INVALID_INDEX UINT32_MAX

namespace Dx12Core
{
	class StaticDescriptorHeap;
	class DescriptorAllocation
	{
	public:

		// Creates a NULL descriptor.
		DescriptorAllocation();

		DescriptorAllocation(
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
			DescriptorIndex index,
			StaticDescriptorHeap* heap);

		// The destructor will automatically free the allocation.
		~DescriptorAllocation();

		// Copies are not allowed.
		DescriptorAllocation(const DescriptorAllocation&) = delete;
		DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

		// Move is allowed.
		DescriptorAllocation(DescriptorAllocation&& allocation);
		DescriptorAllocation& operator=(DescriptorAllocation&& other);

		// Check if this a valid descriptor.
		bool IsNull() const { return this->m_index == UINT_MAX; }
		bool IsShaderVisible() const { return this->m_gpuHandle.ptr != 0; }

		// Get a descriptor at a particular offset in the allocation.
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return this->m_cpuHandle; };
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return this->m_gpuHandle; };

		// Get the number of (consecutive) handles for this allocation.
		DescriptorIndex GetIndex() const { return this->m_index; }

	private:
		// Free the descriptor back to the heap it came from.
		void Free();

	private:
		// The base cpu handle.
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

		DescriptorIndex m_index;

		// A pointer back to the original page where this allocation came from.
		StaticDescriptorHeap* m_heap;
	};
}

