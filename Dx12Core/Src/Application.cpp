#include "Dx12Core\Application.h"
#include "Dx12Core/Log.h"
#include <assert.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

using namespace Dx12Core;


bool ApplicationDx12Base::sGlwfIsInitialzed = false;;

Dx12Core::ApplicationDx12Base::~ApplicationDx12Base()
{
	this->DestoryApplicationWindow();
}

void Dx12Core::ApplicationDx12Base::Initialize(IGraphicsDevice* graphicsDevice)
{
	this->CreateApplicationWindow(WindowProperties());

	this->m_graphicsDevice = graphicsDevice;

	{
		SwapChainDesc desc = {};
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = this->m_windowProperties.Width;
		desc.Height = this->m_windowProperties.Height;
		desc.WindowHandle = glfwGetWin32Window(this->m_window);
		this->m_graphicsDevice->InitializeSwapcChain(desc);
	}

}

void Dx12Core::ApplicationDx12Base::Run()
{
	this->m_previousFrameTimestamp = glfwGetTime();

	this->LoadContent();
	while (!glfwWindowShouldClose(this->m_window))
	{
		glfwPollEvents();

		this->UpdateWindowSize();

		double currFrameTime = glfwGetTime();
		double elapsedTime = currFrameTime - this->m_previousFrameTimestamp;

		// Update Inputs
		if (this->m_isWindowVisible)
		{
			this->m_graphicsDevice->BeginFrame();
			this->Update(elapsedTime);
			this->Render();
			this->m_graphicsDevice->Present();
		}

		// std::this_thread::sleep_for(std::chrono::milliseconds(0));


		this->UpdateAvarageFrameTime(elapsedTime);
		this->m_previousFrameTimestamp = currFrameTime;
	}
}

void Dx12Core::ApplicationDx12Base::Shutdown()
{
	this->GetDevice()->WaitForIdle();
}

void ApplicationDx12Base::CreateApplicationWindow(WindowProperties properties)
{
	LOG_CORE_INFO("Creating window {0} ({1}, {2})", properties.Title, properties.Width, properties.Height);

	if (!sGlwfIsInitialzed)
	{
		int success = glfwInit();
		assert(success);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// TODO Check Flags
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	this->m_window =
		glfwCreateWindow(
			properties.Width,
			properties.Height,
			properties.Title.c_str(),
			nullptr,
			nullptr);

	// glfwSetWindowUserPointer(this->m_window, &this->m_windowState);

	/*
	// Handle Close Event
	glfwSetWindowCloseCallback(this->m_window, [](GLFWwindow* window) {
		WindowState& state = *(WindowState*)glfwGetWindowUserPointer(window);
		state.IsClosing = true;
		});
	*/

	this->m_windowProperties = std::move(properties);
}

void Dx12Core::ApplicationDx12Base::DestoryApplicationWindow()
{
	if (this->m_window)
	{
		LOG_CORE_INFO("Destroying Window");
		glfwDestroyWindow(this->m_window);
	}

	if (sGlwfIsInitialzed)
	{
		glfwTerminate();
		sGlwfIsInitialzed = false;
	}
}

void Dx12Core::ApplicationDx12Base::UpdateWindowSize()
{
	int width;
	int height;
	glfwGetWindowSize(this->m_window, &width, &height);

	if (width == 0 || height == 0)
	{
		// window is minimized
		this->m_isWindowVisible = false;
		return;
	}

	this->m_isWindowVisible = true;

	// TODO: Update back buffers if resize
}

void Dx12Core::ApplicationDx12Base::UpdateAvarageFrameTime(double elapsedTime)
{
	this->m_frameTimeSum += elapsedTime;
	this->m_numberOfAccumulatedFrames += 1;

	if (this->m_frameTimeSum > this->m_averageTimeUpdateInterval && this->m_numberOfAccumulatedFrames > 0)
	{
		this->m_averageFrameTime = this->m_frameTimeSum / double(this->m_numberOfAccumulatedFrames);
		this->m_numberOfAccumulatedFrames = 0;
		this->m_frameTimeSum = 0.0;
	}
}