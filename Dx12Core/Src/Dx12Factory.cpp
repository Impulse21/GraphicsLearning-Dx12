#include"Dx12Core/Dx12Factory.h"

#include "Dx12Core/GraphicsDevice.h"

#define _KB(x) ((size_t) (x) << 10)
#define _MB(x) ((size_t) (x) << 20)
#define _GB(x) ((size_t) (x) << 30)
#define _64KB _KB(64)
#define _1MB _MB(1)
#define _2MB _MB(2)
#define _4MB _MB(4)
#define _8MB _MB(8)
#define _16MB _MB(16)
#define _32MB _MB(32)
#define _64MB _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)

#define BYTE_TO_MB(x) ((size_t) (x) / _MB(1))
#define BYTE_TO_GB(x) ((size_t) (x) / _GB(1))

using namespace Dx12Core;

static const GUID RenderdocUUID = { 0xa7aa6116, 0x9c8d, 0x4bba, { 0x90, 0x83, 0xb4, 0xd8, 0x16, 0xb7, 0x1b, 0x78 } };
static const GUID PixUUID = { 0x9f251514, 0x9d4d, 0x4902, { 0x9d, 0x60, 0x18, 0x98, 0x8a, 0xb7, 0xd4, 0xb5 } };


std::vector<RefCountPtr<IDXGIAdapter1>> Dx12Core::Dx12Factory::EnumerateAdapters(bool includeSoftwareAdapter)
{
	std::vector<RefCountPtr<IDXGIAdapter1>> adapters;

	auto factory = this->CreateFactory();

	auto nextAdapter = [&](uint32_t adapterIndex, RefCountPtr<IDXGIAdapter1>& adapter)
	{
		return factory->EnumAdapterByGpuPreference(
			adapterIndex,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
	};

	RefCountPtr<IDXGIAdapter1> adapter;
	uint32_t gpuIndex = 0;
	for (uint32_t adapterIndex = 0; DXGI_ERROR_NOT_FOUND != nextAdapter(adapterIndex, adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc = {};
		adapter->GetDesc1(&desc);
		if (!includeSoftwareAdapter && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
			continue;

		adapters.push_back(adapter);
	}

	return adapters;
}

RefCountPtr<IDXGIAdapter1> Dx12Core::Dx12Factory::SelectOptimalGpu()
{
	LOG_CORE_INFO("Selecting Optimal GPU");
	RefCountPtr<IDXGIAdapter1> selectedGpu;

	size_t selectedGPUVideoMemeory = 0;
	for (auto adapter : EnumerateAdapters())
	{
		DXGI_ADAPTER_DESC desc = {};
		adapter->GetDesc(&desc);

		std::string name = NarrowString(desc.Description);
		size_t dedicatedVideoMemory = desc.DedicatedVideoMemory;
		size_t dedicatedSystemMemory = desc.DedicatedSystemMemory;
		size_t sharedSystemMemory = desc.SharedSystemMemory;

		LOG_CORE_INFO(
			"\t{0} [VRAM={1}MB, SRAM={2}MB, SharedRAM={3}MB]",
			name,
			BYTE_TO_MB(dedicatedVideoMemory),
			BYTE_TO_MB(dedicatedSystemMemory),
			BYTE_TO_MB(sharedSystemMemory));

		if (!selectedGpu || selectedGPUVideoMemeory < dedicatedVideoMemory)
		{
			selectedGpu = adapter;
			selectedGPUVideoMemeory = dedicatedVideoMemory;
		}
	}

	DXGI_ADAPTER_DESC desc = {};
	selectedGpu->GetDesc(&desc);

	std::string name = NarrowString(desc.Description);
	size_t dedicatedVideoMemory = desc.DedicatedVideoMemory;
	size_t dedicatedSystemMemory = desc.DedicatedSystemMemory;
	size_t sharedSystemMemory = desc.SharedSystemMemory;

	LOG_CORE_INFO(
		"Selected GPU {0} [VRAM={1}MB, SRAM={2}MB, SharedRAM={3}MB]",
		name,
		BYTE_TO_MB(dedicatedVideoMemory),
		BYTE_TO_MB(dedicatedSystemMemory),
		BYTE_TO_MB(sharedSystemMemory));

	return selectedGpu;
}

Dx12Context Dx12Core::Dx12Factory::CreateContext()
{
	Dx12Context context = {};
	context.GpuAdapter = this->SelectOptimalGpu();

	ThrowIfFailed(
		D3D12CreateDevice(
			context.GpuAdapter,
			D3D_FEATURE_LEVEL_11_1,
			IID_PPV_ARGS(&context.Device)));

	/* TODO FIX ME
	RefCountPtr<IUnknown> renderdoc;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, RenderdocUUID, &renderdoc)))
	{
		this->IsUnderGraphicsDebugger |= !!renderdoc;
	}

	RefCountPtr<IUnknown> pix;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, PixUUID, &pix)))
	{
		this->IsUnderGraphicsDebugger |= !!pix;
	}
	*/

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupport5 = {};
	bool hasOptions5 = SUCCEEDED(context.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupport5, sizeof(featureSupport5)));

	D3D12_FEATURE_DATA_D3D12_OPTIONS6 featureSupport6 = {};
	bool hasOptions6 = SUCCEEDED(context.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &featureSupport6, sizeof(featureSupport6)));

	// D3D12_FEATURE_DATA_D3D12_OPTIONS7 featureSupport7 = {};
	bool hasOptions7 = false; // SUCCEEDED(m_Context.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &m_Options7, sizeof(m_Options7)));

	if (SUCCEEDED(context.Device->QueryInterface(&context.Device5)) && hasOptions5)
	{
		context.IsDxrSupported = featureSupport5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
		context.IsRenderPassSupported = featureSupport5.RenderPassesTier >= D3D12_RENDER_PASS_TIER_0;
		// this->IsRayQuerySupported= featureSupport5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1;
	}

	if (hasOptions6)
	{
		context.IsVariableRateShadingSupported = featureSupport6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2;
		// this->m_shadingRateImageTileSize = featureSupport6.ShadingRateImageTileSize;
	}


	if (SUCCEEDED(context.Device->QueryInterface(&context.Device2)) && hasOptions7)
	{
		// this->IsCreateNotZeroedAvailable = true;
		// this->m_meshShadingSupported = feature_support7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;
	}

	static const bool debugEnabled = IsDebuggerPresent();
	if (debugEnabled)
	{
		RefCountPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(context.Device->QueryInterface(&infoQueue)))
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

			D3D12_MESSAGE_SEVERITY severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO,
			};

			D3D12_MESSAGE_ID denyIds[] =
			{
				// This occurs when there are uninitialized descriptors in a descriptor table, even when a
				// shader does not access the missing descriptors.  I find this is common when switching
				// shader permutations and not wanting to change much code to reorder resources.
				D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

				// Triggered when a shader does not export all color components of a render target, such as
				// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
				D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

				// This occurs when a descriptor table is unbound even when a shader does not access the missing
				// descriptors.  This is common with a root signature shared between disparate shaders that
				// don't all need the same types of resources.
				D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

				// RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
				(D3D12_MESSAGE_ID)1008,
			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumSeverities = static_cast<UINT>(std::size(severities));
			filter.DenyList.pSeverityList = severities;
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(denyIds));
			filter.DenyList.pIDList = denyIds;
			infoQueue->PushStorageFilter(&filter);
		}
	}

	context.Factory = this->CreateFactory();
	return context;
}

GraphicsDeviceHandle Dx12Core::Dx12Factory::CreateGraphicsDevice(GraphicsDeviceDesc const& desc, Dx12Context& context)
{
	std::unique_ptr<GraphicsDevice> device = std::make_unique<GraphicsDevice>(desc, context);

	return GraphicsDeviceHandle::Create(device.release());
}

GraphicsDeviceHandle Dx12Core::Dx12Factory::CreateGraphicsDevice(GraphicsDeviceDesc const& desc)
{
	return this->CreateGraphicsDevice(desc, this->CreateContext());
}

void Dx12Core::Dx12Factory::ReportLiveObjects()
{
	static const bool debugEnabled = IsDebuggerPresent();

	if (debugEnabled)
	{
		RefCountPtr<IDXGIDebug1> debugController;
		ThrowIfFailed(
			DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugController)));

		debugController->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL| DXGI_DEBUG_RLO_IGNORE_INTERNAL));

		LOG_CORE_INFO("Reporting live objects. See output window for more info");
	}
}

RefCountPtr<IDXGIFactory6> Dx12Core::Dx12Factory::CreateFactory()
{
	uint32_t flags = 0;
	static const bool debugEnabled = IsDebuggerPresent();

	if (debugEnabled)
	{
		RefCountPtr<ID3D12Debug> debugController;
		ThrowIfFailed(
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

		debugController->EnableDebugLayer();
		flags = DXGI_CREATE_FACTORY_DEBUG;
	}

	RefCountPtr<IDXGIFactory6> factory;
	ThrowIfFailed(
		CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)));

	return factory;
}
