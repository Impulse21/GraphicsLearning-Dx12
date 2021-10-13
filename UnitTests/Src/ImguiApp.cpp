
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

#include "Shaders/BindlessExampleVS_compiled.h"
#include "Shaders/BindlessExamplePS_compiled.h"

#include <DirectXMath.h>
#include "Dx12Core/ImGui/ImGuiRenderer.h"

#include "imgui.h"

using namespace Dx12Core;
using namespace DirectX;


class ImGuiApp : public ApplicationDx12Base
{
public:
	ImGuiApp() = default;

protected:
	void LoadContent() override;
	void Update(double elapsedTime) override;
	void Render() override;

private:
	std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
	TextureHandle m_depthBuffer;
};


CREATE_APPLICATION(ImGuiApp)


void ImGuiApp::LoadContent()
{
	this->m_imguiRenderer = std::make_unique<ImGuiRenderer>();

	this->m_imguiRenderer->Initialize(this->GetWindow(), this->GetDevice());
}

void ImGuiApp::Update(double elapsedTime)
{
	this->m_imguiRenderer->BeginFrame();

	ImGui::ShowDemoWindow();
}

void ImGuiApp::Render()
{
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Clearing Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 0.0f, 1.0f });
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Draw ImGui");

		this->m_imguiRenderer->Draw(gfxContext, this->GetDevice());
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Transition Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	this->GetDevice()->Submit();
}
