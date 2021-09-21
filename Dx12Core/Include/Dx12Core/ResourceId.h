#pragma once

#include <stdint.h>

#define INVALID_KEY ~0u


namespace Dx12Core
{
	const unsigned RESOURCE_ID_INDEX_BITS = 32;
	const unsigned RESOURCE_ID_INDEX_MASK = (1 << RESOURCE_ID_INDEX_BITS) - 1;

	const unsigned RESOURCE_ID_GENERATION_BITS = 8;
	const unsigned RESOURCE_ID_GENERATION_MASK = (1 << RESOURCE_ID_GENERATION_BITS) - 1;

	const unsigned RESOURCE_ID_OWNER_BITS = 8;
	const unsigned RESOURCE_ID_OWNER_MASK = (1 << RESOURCE_ID_OWNER_BITS) - 1;

	// Lower 32 bits are the local index value.
	// upper 32 bits are used for generation value to disinquish entities at the same index. 
	class ResourceId
	{
	public:
		ResourceId()
		{
			this->m_key = INVALID_KEY;
		}

		ResourceId(uint8_t onwerId, uint8_t generation, uint32_t index)
		{
			this->m_key =
				(onwerId | (generation << RESOURCE_ID_INDEX_BITS) | (generation << RESOURCE_ID_INDEX_BITS << RESOURCE_ID_GENERATION_BITS));
		}


		bool IsValid() const { return this->m_key != INVALID_KEY; }
		bool IsNull() const { return this->m_key == INVALID_KEY; }

		uint32_t GetLocalIndex() { return this->m_key & RESOURCE_ID_INDEX_MASK; }
		uint8_t GetGeneration() { return (this->m_key >> RESOURCE_ID_INDEX_BITS) & RESOURCE_ID_GENERATION_MASK; }
		uint8_t GetOwnerId() { return (this->m_key >> RESOURCE_ID_INDEX_BITS >> RESOURCE_ID_GENERATION_BITS) & RESOURCE_ID_OWNER_MASK; }

		__forceinline bool operator==(ResourceId const& rid) const 
		{
			return this->m_key == rid.m_key;
		}

		__forceinline bool operator<(ResourceId const& rid) const
		{
			return this->m_key < rid.m_key;
		}

		__forceinline bool operator<=(ResourceId const& rid) const
		{
			return this->m_key <= rid.m_key;
		}

		__forceinline bool operator>(ResourceId const& rid) const
		{
			return this->m_key > rid.m_key;
		}
		__forceinline bool operator>=(ResourceId const& rid) const
		{
			return this->m_key >= rid.m_key;
		}
		__forceinline bool operator!=(ResourceId const& rid) const
		{
			return this->m_key != rid.m_key;
		}
		operator uint64_t() { return m_key; };

	private:
		uint64_t m_key = INVALID_KEY;
	};
}