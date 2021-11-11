#include "Dx12Core/GraphicsDevice.h"
#include "Dx12Core/Dx12Queue.h"
#include "Dx12DescriptorHeap.h"

// #include "d3d12shader.h"

using namespace Dx12Core;

Dx12Core::GraphicsDevice::GraphicsDevice(GraphicsDeviceDesc desc, Dx12Context& context)
	: m_context(context)
	, m_desc(std::move(desc))
{
	this->m_queues[static_cast<size_t>(CommandQueue::Graphics)] 
		= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_DIRECT);

	if (this->m_desc.EnableComputeQueue)
	{
		this->m_queues[static_cast<size_t>(CommandQueue::Compute)] 
			= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}

	if (this->m_desc.EnableCopyQueue)
	{
		this->m_queues[static_cast<size_t>(CommandQueue::Copy)] 
			= std::make_unique<Dx12Queue>(this->m_context, D3D12_COMMAND_LIST_TYPE_COPY);
	}

	this->m_cpuDescriptorHeaps[DescritporHeapType::Srv_Cbv_Uav] =
		std::make_unique<CpuDescriptorHeap>(
			*this,
			desc.ShaderResourceViewCpuHeapSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	this->m_cpuDescriptorHeaps[DescritporHeapType::Sampler] =
		std::make_unique<CpuDescriptorHeap>(
			*this,
			desc.SamplerHeapCpuSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	this->m_cpuDescriptorHeaps[DescritporHeapType::Rtv] =
		std::make_unique<CpuDescriptorHeap>(
			*this,
			desc.RenderTargetViewHeapSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	this->m_cpuDescriptorHeaps[DescritporHeapType::Dsv] =
		std::make_unique<CpuDescriptorHeap>(
			*this,
			desc.DepthStencilViewHeapSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	this->m_gpuDescritporHeaps[DescritporHeapType::Srv_Cbv_Uav] =
		std::make_unique<GpuDescriptorHeap>(
			*this,
			desc.ShaderResourceViewGpuStaticHeapSize,
			desc.ShaderResourceViewGpuDynamicHeapSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	this->m_gpuDescritporHeaps[DescritporHeapType::Sampler] =
		std::make_unique<GpuDescriptorHeap>(
			*this,
			desc.SamplerHeapGpuSize,
			desc.SamplerHeapGpuSize,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

}

Dx12Core::GraphicsDevice::~GraphicsDevice()
{
	this->WaitForIdle();

	this->m_swapChainTextures.clear();

	this->m_context.Device5.Reset();
	this->m_context.Device2.Reset();
	this->m_context.Device.Reset();
}

void Dx12Core::GraphicsDevice::InitializeSwapcChain(SwapChainDesc const& swapChainDesc)
{
	assert(swapChainDesc.WindowHandle);
	if (!swapChainDesc.WindowHandle)
	{
		LOG_CORE_ERROR("Invalid window handle");
		throw std::runtime_error("Invalid Window Error");
	}

	DXGI_SWAP_CHAIN_DESC1 dx12Desc = {};
	dx12Desc.Width = swapChainDesc.Width;
	dx12Desc.Height = swapChainDesc.Height;
	dx12Desc.Format = swapChainDesc.Format;
	dx12Desc.SampleDesc.Count = 1;
	dx12Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dx12Desc.BufferCount = swapChainDesc.NumBuffers;
	dx12Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dx12Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;


	RefCountPtr<IDXGISwapChain1> tmpSwapChain;
	ThrowIfFailed(
		this->m_context.Factory->CreateSwapChainForHwnd(
			this->m_queues[D3D12_COMMAND_LIST_TYPE_DIRECT]->GetNative(),
			swapChainDesc.WindowHandle,
			&dx12Desc,
			nullptr,
			nullptr,
			&tmpSwapChain));

	ThrowIfFailed(
		tmpSwapChain->QueryInterface(&this->m_swapChain));

	this->m_swapChainDesc = swapChainDesc;

	this->m_frames.resize(this->m_swapChainDesc.NumBuffers);
	this->InitializeRenderTargets();
}

ICommandContext& Dx12Core::GraphicsDevice::BeginContext()
{
	std::scoped_lock(this->m_commandListMutex);
	ContextId contextId = this->m_activeContext++;

	assert(contextId < MAX_COMMAND_CONTEXT);
	if (!this->m_commandContexts[contextId])
	{
		this->m_commandContexts[contextId] = 
			std::make_unique<Dx12CommandContext>(
				this->m_context.Device2,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				std::wstring(L"Command List: ") + std::to_wstring(contextId));
	}
	else
	{
		this->m_commandContexts[contextId]->Reset(this->GetGfxQueue()->GetLastCompletedFence());
	}

	this->m_commandContexts[contextId]->BindHeaps(
		{
			this->m_gpuDescritporHeaps[Srv_Cbv_Uav].get(),
			this->m_gpuDescritporHeaps[Sampler].get()
		});

	return *this->m_commandContexts[contextId];
}

uint64_t Dx12Core::GraphicsDevice::Submit(bool waitForCompletion)
{
	std::scoped_lock(this->m_commandListMutex);
	ContextId cmdLast = this->m_activeContext;
	this->m_activeContext = 0;

	this->m_commandListsToExecute.resize(cmdLast);
	for (ContextId i = 0; i < cmdLast; i++)
	{
		Dx12CommandContext& context = *this->m_commandContexts[i];
		context.Close();
		this->m_commandListsToExecute[i] = context.GetInternal();
	}

	uint64_t fence = this->GetGfxQueue()->ExecuteCommandLists(this->m_commandListsToExecute);

	for (ContextId i = 0; i < cmdLast; i++)
	{
		auto trackedResources = this->m_commandContexts[i]->Executed(fence);
		this->GetCurrentFrame().ReferencedResources.push_back(trackedResources);
	}

	if (waitForCompletion)
	{
		this->GetGfxQueue()->WaitForFence(fence);
	}

	return fence;
}

void Dx12Core::GraphicsDevice::BeginFrame()
{
	uint32_t bufferIndex = this->GetCurrentBackBufferIndex();

	Frame& frame = this->GetCurrentFrame();
	this->GetGfxQueue()->WaitForFence(frame.FrameFence);

	uint64_t completedFence = this->GetGfxQueue()->GetLastCompletedFence();

	// Purge stale resources
	while (!this->m_safeReleaseQueue.empty() && std::get<uint64_t>(this->m_safeReleaseQueue.front()) <= completedFence)
	{
		this->m_safeReleaseQueue.pop_front();
	}

	frame.ReferencedResources.clear();
}

void Dx12Core::GraphicsDevice::Present()
{
	this->m_swapChain->Present(0, 0);

	this->GetCurrentFrame().FrameFence = this->GetGfxQueue()->IncrementFence();

	this->m_frame = (this->m_frame + 1) % this->m_swapChainDesc.NumBuffers;
}

void Dx12Core::GraphicsDevice::WaitForIdle() const
{
	for (auto& queue : this->m_queues)
	{
		if (queue)
		{
			queue->WaitForIdle();
		}
	}
}

DescriptorIndex Dx12Core::GraphicsDevice::GetDescritporIndex(ITexture* texture) const
{
	Texture* internal = SafeCast<Texture*>(texture);

	return internal->ResourceIndex;
}

DescriptorIndex Dx12Core::GraphicsDevice::GetDescritporIndex(IBuffer* buffer) const
{
	Buffer* internal = SafeCast<Buffer*>(buffer);

	return internal->ResourceIndex;
}

TextureHandle Dx12Core::GraphicsDevice::CreateTexture(TextureDesc desc)
{
	// Use unique ptr here to ensure safety until we are able to pass this over to the texture handle
	std::unique_ptr<Texture> internal = std::make_unique<Texture>(this, desc);

	D3D12_CLEAR_VALUE optimizedClearValue = {};
	if (desc.OptmizedClearValue.has_value())
	{
		optimizedClearValue.Format = desc.Format;
		optimizedClearValue.DepthStencil = desc.OptmizedClearValue.value().DepthStencil;
	}
	
	D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;

	if (BindFlags::DepthStencil == (desc.Bindings | BindFlags::DepthStencil))
	{
		resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	ThrowIfFailed(
		this->m_context.Device2->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(
				desc.Format, desc.Width, desc.Height,
				desc.ArraySize,
				desc.MipLevels,
				1,
				0,
				resourceFlags),
			desc.InitialState,
			desc.OptmizedClearValue.has_value() ? &optimizedClearValue : nullptr,
			IID_PPV_ARGS(&internal->D3DResource)
	));

	std::wstring debugName(desc.DebugName.begin(), desc.DebugName.end());
	internal->D3DResource->SetName(debugName.c_str());

	// Create Views
	if (BindFlags::DepthStencil == (desc.Bindings | BindFlags::DepthStencil))
	{
		internal->Dsv = this->GetCpuHeap(Dsv)->Allocate(1);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = desc.Format;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // TODO: use desc.
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		this->m_context.Device2->CreateDepthStencilView(
			internal->D3DResource,
			&dsvDesc,
			internal->Dsv.GetCpuHandle());
	}
	else if (BindFlags::ShaderResource == (desc.Bindings | BindFlags::ShaderResource))
	{
		internal->Srv = this->GetGpuResourceHeap()->Allocate(1);

		internal->ResourceIndex = static_cast<DescriptorIndex>(internal->Srv.GetGpuHandle().ptr / internal->Srv.GetDescriptorSize());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format; // TODO: handle SRGB format
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (desc.Dimension == TextureDimension::TextureCube)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;  // Only 2D textures are supported (this was checked in the calling function).
			srvDesc.TextureCube.MipLevels = desc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;  // Only 2D textures are supported (this was checked in the calling function).
			srvDesc.Texture2D.MipLevels = internal->GetDesc().MipLevels;
		}

		this->m_context.Device2->CreateShaderResourceView(
			internal->D3DResource,
			&srvDesc,
			internal->Srv.GetCpuHandle());
	}

	return TextureHandle::Create(internal.release());
}

TextureHandle Dx12Core::GraphicsDevice::CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native)
{
	// Use unique ptr here to ensure safety until we are able to pass this over to the texture handle
	std::unique_ptr<Texture> internal = std::make_unique<Texture>(this, desc);
	internal->D3DResource = native;

	if ((internal->GetDesc().Bindings & BindFlags::RenderTarget) == BindFlags::RenderTarget)
	{
		internal->Rtv = this->GetCpuHeap(Rtv)->Allocate(1);
		this->m_context.Device2->CreateRenderTargetView(internal->D3DResource, nullptr, internal->Rtv.GetCpuHandle());
	}

	std::wstring debugName(internal->GetDesc().DebugName.begin(), internal->GetDesc().DebugName.end());
	internal->D3DResource->SetName(debugName.c_str());

	return TextureHandle::Create(internal.release());
}

BufferHandle Dx12Core::GraphicsDevice::CreateBuffer(BufferDesc desc)
{
	D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
	if ((desc.BindFlags | BindFlags::UnorderedAccess) == BindFlags::UnorderedAccess)
	{
		resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	std::unique_ptr<Buffer> internal = std::make_unique<Buffer>(std::move(desc));
	// Create a committed resource for the GPU resource in a default heap.
	ThrowIfFailed(
		this->m_context.Device2->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(internal->GetDesc().SizeInBytes, resourceFlags),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&internal->D3DResource)));


	internal->D3DResource->SetName(internal->GetDesc().DebugName.c_str());
	if (BindFlags::VertexBuffer == (internal->GetDesc().BindFlags | BindFlags::VertexBuffer))
	{
		internal->VertexView = {};
		auto& view = internal->VertexView;
		view.BufferLocation = internal->D3DResource->GetGPUVirtualAddress();
		view.StrideInBytes = internal->GetDesc().StrideInBytes;
		view.SizeInBytes = internal->GetDesc().SizeInBytes;
	}
	else if (BindFlags::IndexBuffer == (internal->GetDesc().BindFlags | BindFlags::IndexBuffer))
	{
		auto& view = internal->IndexView;
		view.BufferLocation = internal->D3DResource->GetGPUVirtualAddress();
		view.Format = internal->GetDesc().StrideInBytes == sizeof(uint32_t)
			? DXGI_FORMAT_R32_UINT
			: DXGI_FORMAT_R16_UINT;
		view.SizeInBytes = internal->GetDesc().SizeInBytes;
	}
	else if (BindFlags::ShaderResource == (internal->GetDesc().BindFlags | BindFlags::ShaderResource))
	{
		internal->Srv = this->GetGpuResourceHeap()->Allocate(1);

		internal->ResourceIndex = static_cast<DescriptorIndex>(internal->Srv.GetGpuHandle().ptr / internal->Srv.GetDescriptorSize());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = internal->GetDesc().SizeInBytes/ internal->GetDesc().StrideInBytes;
		srvDesc.Buffer.StructureByteStride = internal->GetDesc().StrideInBytes;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		this->m_context.Device2->CreateShaderResourceView(
			internal->D3DResource,
			&srvDesc,
			internal->Srv.GetCpuHandle());
	}

	return BufferHandle::Create(internal.release());
}

ShaderHandle Dx12Core::GraphicsDevice::CreateShader(ShaderDesc const& desc, const void* binary, size_t binarySize)
{
	auto internal = std::make_unique<Shader>(desc, binary, binarySize);

	LOG_CORE_INFO("Shader Reflection Info %s", desc.debugName.c_str());
	this->CollectShaderParameters(binary, binarySize);

	return ShaderHandle::Create(internal.release());
}

GraphicsPipelineHandle Dx12Core::GraphicsDevice::CreateGraphicPipeline(GraphicsPipelineDesc desc)
{
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	RootSignatureHandle rootSig =
		desc.UseShaderParameters
		? this->CreateRootSignature(
			desc.ShaderParameters.Flags,
			desc.ShaderParameters.Binding,
			desc.ShaderParameters.Bindless)
		: this->CreateRootSignature(desc.RootSignatureDesc);

	pipelineStateStream.pRootSignature = rootSig->D3DRootSignature;

	// TODO:
	// pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(desc.VS->GetByteCode().data(), desc.VS->GetByteCode().size());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(desc.PS->GetByteCode().data(), desc.PS->GetByteCode().size());

	pipelineStateStream.InputLayout = { desc.InputLayout.data(), static_cast<UINT>(desc.InputLayout.size()) };


	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = std::min(8U, (UINT)desc.RenderState.RtvFormats.size());
	for (int i = 0; i < rtvFormats.NumRenderTargets; i < i++)
	{
		rtvFormats.RTFormats[i] = desc.RenderState.RtvFormats[i];
	}

	pipelineStateStream.RTVFormats = rtvFormats;

	if (desc.RenderState.DsvFormat != DXGI_FORMAT_UNKNOWN)
	{
		pipelineStateStream.DSVFormat = desc.RenderState.DsvFormat;
	}

	if (desc.RenderState.RasterizerState)
	{
		pipelineStateStream.RasterizerState = *desc.RenderState.RasterizerState;
	}

	if (desc.RenderState.DepthStencilState)
	{
		pipelineStateStream.DepthStencilState = *desc.RenderState.DepthStencilState;
	}

	if (desc.RenderState.BlendState)
	{
		pipelineStateStream.BlendState = *desc.RenderState.BlendState;
	}

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = 
	{ 
		sizeof(PipelineStateStream),
		&pipelineStateStream
	};

	RefCountPtr<ID3D12PipelineState> pipelineState;
	ThrowIfFailed(
		this->m_context.Device2->CreatePipelineState(
			&pipelineStateStreamDesc,
			IID_PPV_ARGS(&pipelineState)));

	auto graphicsPipelineState = std::make_unique<GraphicsPipeline>(std::move(desc));
	graphicsPipelineState->D3DPipelineState = pipelineState;
	graphicsPipelineState->RootSignature = rootSig;

	if (desc.UseShaderParameters && desc.ShaderParameters.Bindless)
	{
		graphicsPipelineState->HasBindlessParamaters = true;
		graphicsPipelineState->bindlessResourceTable = this->GetGpuResourceHeap()->GetNativeHeap()->GetGPUDescriptorHandleForHeapStart();
	}

	return GraphicsPipelineHandle::Create(graphicsPipelineState.release());
}

RootSignatureHandle Dx12Core::GraphicsDevice::CreateRootSignature(RootSignatureDesc& desc)
{
	auto d3dRootSig = this->CreateD3DRootSignature(std::move(desc.BuildDx12Desc()));
	std::unique_ptr<RootSignature> rootSignature = std::make_unique<RootSignature>(d3dRootSig);

	return RootSignatureHandle::Create(rootSignature.release());
}

RootSignatureHandle Dx12Core::GraphicsDevice::CreateRootSignature(
	D3D12_ROOT_SIGNATURE_FLAGS flags,
	ShaderParameterLayout* shaderParameter,
	BindlessShaderParameterLayout* bindlessLayout)
{
	std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;

	if (shaderParameter)
	{
		// Memcopy, yes please
		parameters.resize(shaderParameter->Parameters.size());
		std::memcpy(parameters.data(), shaderParameter->Parameters.data(), sizeof(CD3DX12_ROOT_PARAMETER1) * parameters.size());

		staticSamplers.resize(shaderParameter->StaticSamplers.size());
		std::memcpy(staticSamplers.data(), shaderParameter->StaticSamplers.data(), sizeof(CD3DX12_STATIC_SAMPLER_DESC) * staticSamplers.size());
	}

	uint32_t bindlessRootParameterOffset = 0;
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges;
	if (bindlessLayout)
	{
		constexpr D3D12_DESCRIPTOR_RANGE_FLAGS flags =
			D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
		bindlessRootParameterOffset = static_cast<uint32_t>(parameters.size());
		descriptorRanges.reserve(bindlessLayout->Parameters.size());

		for (auto& param : bindlessLayout->Parameters)
		{
			assert(param.Type != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, "Not Tested");
			CD3DX12_DESCRIPTOR_RANGE1& range = descriptorRanges.emplace_back();
			range.Init(
				param.Type,
				bindlessLayout->MaxCapacity,
				param.BaseShaderRegister,
				param.RegisterSpace,
				flags,
				bindlessLayout->FirstSlot);
		}

		CD3DX12_ROOT_PARAMETER1& parameter = parameters.emplace_back();
		parameter.InitAsDescriptorTable(
			descriptorRanges.size(),
			descriptorRanges.data(),
			bindlessLayout->Visibility);
	}

	D3D12_ROOT_SIGNATURE_DESC1 desc = {};
	desc.NumParameters = static_cast<UINT>(parameters.size());
	desc.pParameters = parameters.data();
	desc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
	desc.pStaticSamplers = staticSamplers.data();
	desc.Flags = flags;

	auto d3dRootSig = this->CreateD3DRootSignature(std::move(desc));
	std::unique_ptr<RootSignature> rootSignature = std::make_unique<RootSignature>(d3dRootSig);
	rootSignature->BindlessRootParameterOffset = bindlessRootParameterOffset;
	return RootSignatureHandle::Create(rootSignature.release());
}

RefCountPtr<ID3D12RootSignature> Dx12Core::GraphicsDevice::CreateD3DRootSignature(D3D12_ROOT_SIGNATURE_DESC1&& rootSigDesc)
{
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC dx12RootSigDesc = {};
	dx12RootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	dx12RootSigDesc.Desc_1_1 = rootSigDesc;


	// Serialize the root signature
	RefCountPtr<ID3DBlob> serializedRootSignatureBlob;
	RefCountPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(
		::D3DX12SerializeVersionedRootSignature(
			&dx12RootSigDesc,
			this->m_context.FeatureDataRootSignature.HighestVersion,
			&serializedRootSignatureBlob,
			&errorBlob));

	// Create the root signature.
	RefCountPtr<ID3D12RootSignature> dx12RootSig = nullptr;
	ThrowIfFailed(
		this->m_context.Device2->CreateRootSignature(
			0,
			serializedRootSignatureBlob->GetBufferPointer(),
			serializedRootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&dx12RootSig)));

	return dx12RootSig;
}

void Dx12Core::GraphicsDevice::CollectShaderParameters(const void* binary, size_t binarySize)
{
	/*
	DxcBuffer reflectionData;
	reflectionData.Encoding = DXC_CP_ACP;
	reflectionData.Ptr = binary;
	reflectionData.Size = (SIZE_T)binarySize;

	RefCountPtr<ID3D12ShaderReflection> reflection;
	ThrowIfFailed(
		this->m_context.dxcUtils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection)));

	D3D12_SHADER_DESC shaderDesc;
	ThrowIfFailed(
		reflection->GetDesc(&shaderDesc));

	
	LOG_CORE_INFO("\tNumber bound desources: %i", shaderDesc.BoundResources);
	LOG_CORE_INFO("\tNumber Input Parameters: %i", shaderDesc.InputParameters);
	*/
}

void Dx12Core::GraphicsDevice::InitializeRenderTargets()
{
	this->m_swapChainTextures.resize(this->m_swapChainDesc.NumBuffers);
	for (UINT i = 0; i < this->m_swapChainDesc.NumBuffers; i++)
	{
		RefCountPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(
			this->m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		auto textureDesc = TextureDesc();
		textureDesc.Dimension = TextureDimension::Texture2D;
		textureDesc.Format = this->m_swapChainDesc.Format;
		textureDesc.Width = this->m_swapChainDesc.Width;
		textureDesc.Height = this->m_swapChainDesc.Height;
		textureDesc.Bindings = BindFlags::RenderTarget;

		this->m_swapChainTextures[i] = this->CreateTextureFromNative(textureDesc, backBuffer);
	}
}
