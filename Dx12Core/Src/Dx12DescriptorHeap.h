#pragma once

#include "Dx12Core/Dx12Common.h"

#include "DescriptorAllocation.h"

namespace Dx12Core
{

	class StaticDescriptorHeap
	{
	public:
		StaticDescriptorHeap(
			Dx12Context const& context,
			D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			uint32_t numDescriptorsInHeap,
			bool isShaderVisible = false);

		operator ID3D12DescriptorHeap* () const { return this->m_heap; }
		ID3D12DescriptorHeap* GetNative() const { return this->m_heap; }

		bool IsShaderVisible() const { return this->m_isShaderVisible; }

		DescriptorAllocation AllocateDescriptor();
		void Release(DescriptorAllocation&& allocation);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(DescriptorIndex index = 0);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(DescriptorIndex index = 0);

		D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return this->m_heapType; }

	private:
		struct DescriptorIndexPool
		{
			DescriptorIndexPool() = default;
			DescriptorIndexPool(size_t numIndices)
			{
				this->m_elements.resize(numIndices);
				Reset();
			}

			auto& operator[](size_t index) { return this->m_elements[index]; }

			const auto& operator[](size_t index) const { return this->m_elements[index]; }

			void Reset()
			{
				this->m_freeStart = 0;
				this->m_numActiveElements = 0;
				for (size_t i = 0; i < this->m_elements.size(); ++i)
				{
					this->m_elements[i] = i + 1;
				}
			}

			// Removes the first element from the free list and returns its index
			DescriptorIndex Allocate()
			{
				assert(this->m_numActiveElements < this->m_elements.size() && "Consider increasing the size of the pool");
				this->m_numActiveElements++;
				DescriptorIndex index = this->m_freeStart;
				this->m_freeStart = this->m_elements[index];
				return index;
			}

			void Release(DescriptorIndex index)
			{
				this->m_numActiveElements--;
				this->m_elements[index] = this->m_freeStart;
				this->m_freeStart = index;
			}

		private:
			std::vector<DescriptorIndex> m_elements;
			DescriptorIndex m_freeStart;
			size_t m_numActiveElements;
		};

	private:
		const Dx12Context& m_context;
		const D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
		const bool m_isShaderVisible;
		D3D12_DESCRIPTOR_HEAP_DESC m_desc;

		DescriptorIndexPool m_descriptorIndexPool;
		RefCountPtr<ID3D12DescriptorHeap> m_heap;

		D3D12_CPU_DESCRIPTOR_HANDLE	m_cpuStartHandle = { 0 };
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStartHandle = { 0 };
		UINT m_descriptorHandleIncrementSize;
	};
}

