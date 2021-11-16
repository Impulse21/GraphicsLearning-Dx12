#include "Dx12Resources.h"

#include "Dx12Core/GraphicsDevice.h"
#include "BindlessDescriptorTable.h"

using namespace Dx12Core;

Dx12Core::Texture::~Texture()
{
	// Free Descriptors
	if (BindlessIndex != INVALID_DESCRIPTOR_INDEX)
	{
		this->Device->GetBindlessTable()->Free(BindlessIndex);
	}

	if (!Rtv.IsNull())
	{
		this->Device->GetCpuHeap(DescritporHeapType::Rtv)->FreeAllocation(std::move(Rtv));
	}

	if (!Dsv.IsNull())
	{
		this->Device->GetCpuHeap(DescritporHeapType::Dsv)->FreeAllocation(std::move(Dsv));
	}

	if (!Srv.IsNull())
	{
		this->Device->GetCpuHeap(Srv_Cbv_Uav)->FreeAllocation(std::move(Srv));
	}
};
