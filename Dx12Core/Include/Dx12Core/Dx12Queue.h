#pragma once

#include "Dx12Core/Dx12Common.h"

namespace Dx12Core
{
	class Dx12Queue
	{
	public:
		Dx12Queue(D3D12_COMMAND_LIST_TYPE type);

		D3D12_COMMAND_LIST_TYPE GetType() const { return this->m_type; }

	private:
		const D3D12_COMMAND_LIST_TYPE m_type;
		RefCountPtr<ID3D12CommandQueue> m_queue;
	};
}

