
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


// CREATE_APPLICATION(TriangleApp)

int main()
{
	Dx12Core::Log::Initialize();

	GraphicsDeviceDesc desc = {};
	desc.EnableCopyQueue = true;
	desc.EnableComputeQueue = true;

	auto graphicsDevice = Dx12Factory::GetInstance().CreateGraphicsDevice(desc);
	{
		std::unique_ptr<Dx12Core::ApplicationDx12Base> app = std::make_unique<TriangleApp>();
		app->Initialize(graphicsDevice);
		app->Run();
		app->Shutdown();
	}
	graphicsDevice.Reset();
	Dx12Factory::GetInstance().ReportLiveObjects();
	return 0;
}

void TriangleApp::Render()
{
	/*
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 1.0f, 1.0f });

	gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	this->GetDevice()->Submit();
	*/
}
