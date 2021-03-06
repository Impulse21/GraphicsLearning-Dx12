#pragma once

#include <deque>
#include <memory>
#include "Dx12Core/Dx12Common.h"

#define KB(x)   ((size_t) (x) << 10)
#define MB(x)   ((size_t) (x) << 20)

namespace Dx12Core
{
	class UploadBuffer
	{
	public:
		struct Allocation
		{
			void* Cpu;
			D3D12_GPU_VIRTUAL_ADDRESS Gpu;
		};
		explicit UploadBuffer(RefCountPtr<ID3D12Device2> device, size_t pageSize = MB(2));

		Allocation Allocate(size_t sizeInBytes, size_t alignment);

		void Reset();

		size_t GetPageSize() { this->m_pageSize; }

	private:
		struct Page
		{
			Page(RefCountPtr<ID3D12Device2> device, size_t sizeInBytes);
			~Page();

			bool HasSpace(size_t sizeInBytes, size_t alignment) const;

			Allocation Allocate(size_t sizeInBytes, size_t aligmnet);

			void Reset();

		private:
			RefCountPtr<ID3D12Resource> m_d3dResource;

			void* m_cpuPtr;
			D3D12_GPU_VIRTUAL_ADDRESS m_gpuPtr;

			size_t m_pageSize;
			size_t m_offset;
		};

	private:
		using PagePool = std::deque<std::shared_ptr<Page>>;
		std::shared_ptr<Page> RequestPage();

	private:
		RefCountPtr<ID3D12Device2> m_device;
		PagePool m_pagePool;
		PagePool m_availablePages;

		std::shared_ptr<Page> m_currentPage;

		size_t m_pageSize;
	};
}

