#pragma once

#include "Dx12Common.h"
#include "Dx12Core/GraphicsDevice.h"

namespace Dx12Core
{
	class Dx12Factory
	{
	public:
		static Dx12Factory& GetInstance()
		{
			static Dx12Factory instance;
			return instance;
		}

		std::vector<RefCountPtr<IDXGIAdapter1>> EnumerateAdapters(bool includeSoftwareAdapter = false);

		RefCountPtr<IDXGIAdapter1> SelectOptimalGpu();

		Dx12Context CreateContext();

		GraphicsDeviceHandle CreateGraphicsDevice(GraphicsDeviceDesc const& desc);

	protected:
		Dx12Factory();
		~Dx12Factory() = default;

	private:
		RefCountPtr<IDXGIFactory6> m_dxgiFactory;
	};
}

