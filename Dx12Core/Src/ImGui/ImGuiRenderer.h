#pragma once

#include "Dx12Core/Dx12Core.h"

struct ImGuiContext;
struct GLFWwindow;

namespace Dx12Core
{
	class ImGuiRenderer
	{
	public:
		ImGuiRenderer() = default;
		~ImGuiRenderer();

		void Initialize(GLFWwindow* glfwWindow, IGraphicsDevice* device);

	private:
		void CreatePso();

	private:
		ImGuiContext* m_imguiContext;
		TextureHandle m_fontTexture;
		GraphicsPipelineHandle m_pso;
	};
}

