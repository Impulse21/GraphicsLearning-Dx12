#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"

#include "DescriptorAllocation.h"

namespace Dx12Core
{
	class GraphicsDevice;
	class Texture : public RefCounter<ITexture>
	{
	public:
		Texture(GraphicsDevice* device, TextureDesc desc)
			: m_desc(std::move(desc))
			, Device(device)
		{
		}

		~Texture() = default;

		RefCountPtr<ID3D12Resource> D3DResource = nullptr;
		DescriptorAllocation Rtv = {};

		GraphicsDevice* Device;

		const TextureDesc& GetDesc() const override { return this->m_desc; }

	private:
		TextureDesc m_desc;
	};
}
