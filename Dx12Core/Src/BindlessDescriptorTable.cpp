#include "BindlessDescriptorTable.h"

using namespace Dx12Core;

Dx12Core::BindlessDescriptorTable::~BindlessDescriptorTable()
{
}

DescriptorIndex Dx12Core::BindlessDescriptorTable::Allocate()
{
	return this->m_descriptorIndexPool.Allocate();
}

void Dx12Core::BindlessDescriptorTable::Free(DescriptorIndex index)
{
	this->m_descriptorIndexPool.Release(index);
}
