#pragma once

#include "Dx12Core.h"

#include "Dx12Common.h"

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

		GraphicsDeviceHandle CreateGraphicsDevice(GraphicsDeviceDesc const& desc, Dx12Context& context);
		GraphicsDeviceHandle CreateGraphicsDevice(GraphicsDeviceDesc const& desc);

		void ReportLiveObjects();

	protected:
		Dx12Factory() = default;
		~Dx12Factory() = default;

		RefCountPtr<IDXGIFactory6> CreateFactory();
	};
}

