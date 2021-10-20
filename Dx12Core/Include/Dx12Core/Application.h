#pragma once

#include "Dx12Core/Dx12Core.h"

#include "Log.h"
#include "GLFW/glfw3.h"
#include <string>
#include <memory>

#include "Dx12Core/Dx12Factory.h"
#include "Dx12Core/ResourceStore.h"

namespace Dx12Core
{
	using WindowFlags = uint32_t;

	enum WindowOptions
	{
		None = 0,
		DisableFullscreen = 0x01,
		DisableResize = 0x02,
	};

	struct WindowProperties
	{
		uint32_t Width;
		uint32_t Height;
		std::string Title;

		WindowFlags Flags;

		WindowProperties()
			: Title("Dx12 Graphics Learning")
			, Width(1280)
			, Height(720)
			, Flags(WindowOptions::None)
		{}

		WindowProperties(std::string const& title)
			: Title(title)
			, Width(1280)
			, Height(720)
			, Flags(WindowOptions::None)
		{}

		WindowProperties(std::string const& title, uint32_t width, uint32_t height)
			: Title(title)
			, Width(width)
			, Height(height)
			, Flags(WindowOptions::None)
		{}
	};

	class IGraphicsDevice;
	class ApplicationDx12Base
	{
	public:
		ApplicationDx12Base() = default;
		virtual ~ApplicationDx12Base();

		void Initialize(IGraphicsDevice* graphicsDevice);
		void Run();
		void Shutdown();

	protected:
		struct FrameStatistics
		{
			double PreviousFrameTimestamp = 0.0f;
			double AverageFrameTime = 0.0;
			double AverageTimeUpdateInterval = 0.5;
			double FrameTimeSum = 0.0;
			int NumberOfAccumulatedFrames = 0;
		};

	protected:
		virtual void LoadContent() = 0;
		virtual void Update(double elapsedTime) = 0;
		virtual void Render() = 0;

		IGraphicsDevice* GetDevice() { return this->m_graphicsDevice; }
		TextureResourceStore* GetTextureStore() { return this->m_textureStore.get(); }
		GLFWwindow* GetWindow() { return this->m_window; }
		const FrameStatistics& GetFrameStats() { return this->m_frameStatistics; }

	private:
		void CreateApplicationWindow(WindowProperties properties);
		void DestoryApplicationWindow();
		void UpdateWindowSize();
		void UpdateAvarageFrameTime(double elapsedTime);

	private:
		static bool sGlwfIsInitialzed;
		WindowProperties m_windowProperties;
		GLFWwindow* m_window;

		IGraphicsDevice* m_graphicsDevice;
		std::unique_ptr<TextureResourceStore> m_textureStore;

		bool m_isWindowVisible = true;
		FrameStatistics m_frameStatistics = {};
		uint32_t m_frameIndex = 0;
	};
}


#define MAIN_FUNCTION() int main()

#define CREATE_APPLICATION( app_Class )																	\
	MAIN_FUNCTION()																						\
	{																									\
	   Dx12Core::Log::Initialize();																		\
																										\
	   GraphicsDeviceDesc desc = {};																	\
	   desc.EnableCopyQueue = true;																		\
	   desc.EnableComputeQueue = true;																	\
																										\
	   auto graphicsDevice = Dx12Factory::GetInstance().CreateGraphicsDevice(desc);						\
       {																								\
			std::unique_ptr<Dx12Core::ApplicationDx12Base> app = std::make_unique<app_Class>();			\
			app->Initialize(graphicsDevice);															\
			app->Run();																					\
			app->Shutdown();																			\
	   } \
		graphicsDevice.Reset();\
		Dx12Factory::GetInstance().ReportLiveObjects();													\
	   return 0;																						\
	}																									\
