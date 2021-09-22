
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

using namespace Dx12Core;

class TriangleApp : public ApplicationDx12Base
{
public:
	TriangleApp() = default;

protected:
	void Update(double elapsedTime) override {};
	void Render() override;
};


CREATE_APPLICATION(TriangleApp)

void TriangleApp::Render()
{
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 1.0f, 1.0f });

	gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	this->GetDevice()->Submit();
}
