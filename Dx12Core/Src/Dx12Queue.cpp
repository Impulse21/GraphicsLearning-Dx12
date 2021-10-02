#include "Dx12Core/Dx12Queue.h"

using namespace Dx12Core;

Dx12Queue::Dx12Queue(Dx12Context const& context, D3D12_COMMAND_LIST_TYPE type)
	: m_context(context)
	, m_type(type)
	, m_allocatorPool(context.Device2, type)
{
	// Create Command Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = this->m_type;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	ThrowIfFailed(
		this->m_context.Device2->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->m_queueDx12)));

	// Create Fence
	ThrowIfFailed(
		this->m_context.Device2->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->m_fence)));
	this->m_fence->SetName(L"Dx12CommandQueue::Dx12CommandQueue::Fence");

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		this->m_queueDx12->SetName(L"Direct Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		this->m_queueDx12->SetName(L"Copy Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		this->m_queueDx12->SetName(L"Compute Command Queue");
		break;
	}

	this->m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_fenceEvent && "Failed to create fence event handle.");

	this->m_lastCompletedFenceValue = this->m_fence->GetCompletedValue();
	this->m_nextFenceValue = this->m_lastCompletedFenceValue + 1;
}

Dx12Queue::~Dx12Queue()
{
	if (this->m_queueDx12 == nullptr)
	{
		return;
	}

	CloseHandle(this->m_fenceEvent);
}

uint64_t Dx12Core::Dx12Queue::ExecuteCommandLists(std::vector<ID3D12CommandList*> const& commandLists)
{
	this->m_queueDx12->ExecuteCommandLists(commandLists.size(), commandLists.data());

	return this->IncrementFence();
}

uint64_t Dx12Queue::IncrementFence()
{
	std::scoped_lock _(this->m_fenceMutex);
	this->m_queueDx12->Signal(this->m_fence.Get(), this->m_nextFenceValue);
	return this->m_nextFenceValue++;
}

bool Dx12Queue::IsFenceComplete(uint64_t fenceValue)
{
	// Avoid querying the fence value by testing against the last one seen.
	// The max() is to protect against an unlikely race condition that could cause the last
	// completed fence value to regress.
	if (fenceValue > this->m_lastCompletedFenceValue)
	{
		this->m_lastCompletedFenceValue = std::max(this->m_lastCompletedFenceValue, this->m_fence->GetCompletedValue());
	}

	return fenceValue <= this->m_lastCompletedFenceValue;
}

void Dx12Queue::WaitForFence(uint64_t fenceValue)
{
	if (this->IsFenceComplete(fenceValue))
	{
		return;
	}

	// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
	// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
	// the fence can only have one event set on completion, then thread B has to wait for 
	// 100 before it knows 99 is ready.  Maybe insert sequential events?
	{
		std::scoped_lock _(this->m_eventMutex);

		LOG_CORE_INFO("Waiting on fence");
		this->m_fence->SetEventOnCompletion(fenceValue, this->m_fenceEvent);
		WaitForSingleObject(this->m_fenceEvent, INFINITE);
		this->m_lastCompletedFenceValue = fenceValue;

		LOG_CORE_INFO("Waiting Finished");
	}
}

uint64_t Dx12Core::Dx12Queue::GetLastCompletedFence()
{
	std::scoped_lock _(this->m_fenceMutex);
	return this->m_lastCompletedFenceValue;
}
