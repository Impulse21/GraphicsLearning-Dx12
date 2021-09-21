#pragma once

#include "Dx12Core/ResourceId.h"
#include <vector>
#include <deque>
#include <mutex>

#define MINIMUM_FREE_INDICES 1024
namespace Dx12Core
{
	template<typename T, bool IsThreadSafe = true>
	class ResourceIdOwner
	{
	public:
		ResourceIdOwner(uint32_t numValidEntires = 1000)
		{
			this->m_generation.reserve(numValidEntires);
			this->m_data.reserve(numValidEntires);
			this->m_ownerId = ++OwnerIdCounter;
		}

		bool Owns(ResourceId rid)
		{
			const uint32_t index = rid.GetLocalIndex();

			if (this->m_ownerId != rid.GetOwnerId())
			{
				return false;
			}

			if (this->m_generation.size() <= index)
			{
				return false;
			}

			if (this->m_generation[index] != rid.GetGeneration())
			{
				return false;
			}

			return true;
		}

		std::vector<ResourceId> GetOwnedList()
		{
			if (IsThreadSafe)
			{
				this->m_mutex.lock();
			}

			std::vector<ResourceId> retVal;
			for (int i = 0; i < this->m_generation.size(); i++)
			{
				if (std::find(this->m_freeIndices.begin(), this->m_freeIndices.end(), i) == this->m_freeIndices.end())
				{
					retVal.push_back(ResourceId(this->m_ownerId, this->m_generation[i], i));
				}
			}

			if (IsThreadSafe)
			{
				this->m_mutex.unlock();
			}

			return retVal;
		}

		T* GetOrNull(ResourceId rid)
		{
			if (IsThreadSafe)
			{
				this->m_mutex.lock();
			}

			const uint32_t index = rid.GetLocalIndex();

			if (!this->Owns(rid))
			{
				return nullptr;
			}


			T* retVal = &this->m_data[index];
			if (IsThreadSafe)
			{
				this->m_mutex.unlock();
			}

			return retVal;
		}

		ResourceId Create()
		{
			this->Create(T());
		}

		ResourceId Create(T const& initData)
		{
			if (IsThreadSafe)
			{
				this->m_mutex.lock();
			}

			uint32_t index = INVALID_KEY;
			if (this->m_freeIndices.size() > MINIMUM_FREE_INDICES)
			{
				index = this->m_freeIndices.front();
				this->m_freeIndices.pop_front();
			}
			else
			{
				this->m_generation.push_back(0);
				index = this->m_generation.size() - 1;
			}

			this->m_data[index] = initData;

			ResourceId retVal = ResourceId(this->m_ownerId, this->m_generation[index], index);

			if (IsThreadSafe)
			{
				this->m_mutex.unlock();
			}
			return retVal;
		}

		void Free(ResourceId rid)
		{
			if (IsThreadSafe)
			{
				this->m_mutex.lock();
			}

			const uint32_t index = rid.GetLocalIndex();
			++this->m_generation[index];
			this->m_freeIndices.push_back(index);

			if (IsThreadSafe)
			{
				this->m_mutex.unlock();
			}
		}

	private:
		std::vector<uint32_t> m_generation;
		std::vector<T> m_data;

		std::deque<uint32_t> m_freeIndices;

		std::mutex m_mutex;

		uint8_t m_ownerId = 0;

	private:
		std::atomic<uint8_t> OwnerIdCounter;
	};
}