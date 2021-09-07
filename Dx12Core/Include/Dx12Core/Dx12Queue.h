#pragma once

#include "Dx12Core/Dx12Common.h"

namespace Dx12Core
{
	class Dx12Queue
	{
	public:
		Dx12Queue(Dx12Context const& context, D3D12_COMMAND_LIST_TYPE type);
		~Dx12Queue();

		D3D12_COMMAND_LIST_TYPE GetType() const { return this->m_type; }
		
		uint64_t IncrementFence();
		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);
		void WaitForIdle() { this->WaitForFence(this->IncrementFence()); }

		operator ID3D12CommandQueue* () const { return this->m_queueDx12.Get(); }

		ID3D12CommandQueue* GetNative() { return this->m_queueDx12.Get(); }

	private:
		const Dx12Context& m_context;
		const D3D12_COMMAND_LIST_TYPE m_type;
		RefCountPtr<ID3D12CommandQueue> m_queueDx12;
		RefCountPtr<ID3D12Fence> m_fence;

		uint64_t m_nextFenceValue;
		uint64_t m_lastCompletedFenceValue;

		std::mutex m_fenceMutex;
		std::mutex m_eventMutex;
		HANDLE m_fenceEvent;
	};
}

