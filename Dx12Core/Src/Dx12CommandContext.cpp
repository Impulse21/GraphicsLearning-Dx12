#include "Dx12CommandContext.h"

#include <pix.h>

#include <optional>
#include "Dx12DescriptorHeap.h"
#include "DescriptorHeap.h"

// helper function for texture subresource calculations
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx
uint32_t CalcSubresource(uint32_t mipSlice, uint32_t arraySlice, uint32_t planeSlice, uint32_t mipLevels, uint32_t arraySize)
{
	return mipSlice + (arraySlice * mipLevels) + (planeSlice * mipLevels * arraySize);
}

using namespace Dx12Core;
Dx12CommandContext::Dx12CommandContext(
	RefCountPtr<ID3D12Device2> device,
	D3D12_COMMAND_LIST_TYPE type,
	std::wstring const& debugName)
	: m_allocatorPool(device, type)
	, m_device(device)
{
	this->m_allocator = this->m_allocatorPool.RequestAllocator(0);

	ThrowIfFailed(
		device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			this->m_allocator,
			nullptr,
			IID_PPV_ARGS(&this->m_internalList)));

	this->m_trackedResources = std::make_shared<ReferencedResources>();

	this->m_internalList->SetName(debugName.c_str());
	
	this->m_uploadBuffer = std::make_unique<UploadBuffer>(device);

	this->m_cbvSrvUavBindlessTable = D3D12_GPU_DESCRIPTOR_HANDLE();
}

void Dx12Core::Dx12CommandContext::Reset(uint64_t completedFenceValue)
{
	assert(this->m_allocator == nullptr);

	this->m_allocator = this->m_allocatorPool.RequestAllocator(completedFenceValue);

	this->m_internalList->Reset(this->m_allocator, nullptr);

	this->m_trackedResources = std::make_shared<ReferencedResources>();
	this->m_cbvSrvUavBindlessTable = D3D12_GPU_DESCRIPTOR_HANDLE();
	this->m_uploadBuffer->Reset();
}

void Dx12Core::Dx12CommandContext::Close()
{
	this->m_internalList->Close();
}

std::shared_ptr<ReferencedResources> Dx12Core::Dx12CommandContext::Executed(uint64_t fenceValue)
{
	this->m_allocatorPool.DiscardAllocator(fenceValue, this->m_allocator);
	this->m_allocator = nullptr;
	auto retVal = this->m_trackedResources;
	this->m_trackedResources.reset();
	return retVal;
}

void Dx12Core::Dx12CommandContext::BindHeaps(std::array<StaticDescriptorHeap*, 2> const& shaderHeaps)
{
	std::vector<ID3D12DescriptorHeap*> heaps;
	for (auto* heap : shaderHeaps)
	{
		if (heap)
		{
			if (heap->GetHeapType() == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			{
				this->m_cbvSrvUavBindlessTable = heap->GetGpuHandle();
			}

			heaps.push_back(heap->GetNative());
		}
	}

	this->m_internalList->SetDescriptorHeaps(heaps.size(), heaps.data());
}

void Dx12Core::Dx12CommandContext::BindHeaps(std::array<GpuDescriptorHeap*, 2> const& shaderHeaps)
{
	std::vector<ID3D12DescriptorHeap*> heaps;
	for (auto* heap : shaderHeaps)
	{
		if (heap)
		{
			heaps.push_back(heap->GetNativeHeap());
		}
	}

	this->m_internalList->SetDescriptorHeaps(heaps.size(), heaps.data());
}

void Dx12Core::Dx12CommandContext::TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	Texture* internal = SafeCast<Texture*>(texture);
	assert(internal);
	this->m_trackedResources->Resources.push_back(internal);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		internal->D3DResource,
		beforeState, afterState);

	this->m_internalList->ResourceBarrier(1, &barrier);
}

void Dx12Core::Dx12CommandContext::TransitionBarrier(IBuffer* buffer, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	Buffer* internal = SafeCast<Buffer*>(buffer);
	assert(internal);
	this->m_trackedResources->Resources.push_back(internal);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		internal->D3DResource,
		beforeState, afterState);

	this->m_internalList->ResourceBarrier(1, &barrier);
}

void Dx12Core::Dx12CommandContext::ClearTextureFloat(ITexture* texture, Color const& clearColour)
{
	Texture* internal = SafeCast<Texture*>(texture);
	assert(internal);

	this->m_trackedResources->Resources.push_back(internal);
	if ((internal->GetDesc().Bindings & BindFlags::RenderTarget) == BindFlags::RenderTarget)
	{
		this->m_internalList->ClearRenderTargetView(internal->Rtv.GetCpuHandle(), &clearColour.R, 0, nullptr);
	}
}

void Dx12Core::Dx12CommandContext::ClearDepthStencilTexture(ITexture* depthStencil, bool clearDepth, float depth, bool clearStencil, uint8_t stencil)
{
	Texture* internal = SafeCast<Texture*>(depthStencil);

	D3D12_CLEAR_FLAGS flags = {};
	if (clearDepth)
	{
		flags |= D3D12_CLEAR_FLAG_DEPTH;
	}

	if (clearStencil)
	{
		flags |= D3D12_CLEAR_FLAG_STENCIL;
	}

	this->m_internalList->ClearDepthStencilView(
		internal->Dsv.GetCpuHandle(),
		flags,
		depth,
		stencil,
		0,
		nullptr);
}

void Dx12Core::Dx12CommandContext::SetGraphicsState(GraphicsState& state)
{
	GraphicsPipeline* pipeline = SafeCast<GraphicsPipeline*>(state.PipelineState);
	this->m_internalList->SetPipelineState(pipeline->D3DPipelineState);

	// Root Signature without casting? Maybe keep a week reference in the Graphics pipline state object;
	this->m_internalList->SetGraphicsRootSignature(pipeline->RootSignature->D3DRootSignature);

	if (pipeline->HasBindlessParamaters)
	{
		this->m_internalList->SetGraphicsRootDescriptorTable(
			pipeline->RootSignature->BindlessRootParameterOffset, pipeline->bindlessResourceTable);

		// TODO: Samplers
	}
	

	switch (pipeline->GetDesc().PrimitiveTopologyType)
	{
	case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
		this->m_internalList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	}

	CD3DX12_VIEWPORT dx12Viewports[16] = {};
	UINT numViewports = state.Viewports.size();
	for (int i = 0; i < numViewports; i++)
	{
		Viewport& viewport = state.Viewports[i];
		dx12Viewports[i] = CD3DX12_VIEWPORT(
			viewport.MinX,
			viewport.MinY,
			viewport.GetWidth(),
			viewport.GetHeight(),
			viewport.MinZ,
			viewport.MaxZ);
	}
	this->m_internalList->RSSetViewports(numViewports, dx12Viewports);

	this->BindScissorRects(state.ScissorRect);
	
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	for (auto renderTarget : state.RenderTargets)
	{
		Texture* t = SafeCast<Texture*>(renderTarget.Get());

		renderTargetViews.push_back(t->Rtv.GetCpuHandle());
		this->m_trackedResources->Resources.push_back(renderTarget);
	}

	bool hasDepth = false;
	D3D12_CPU_DESCRIPTOR_HANDLE depthView = {};
	if (state.DepthStencil)
	{
		Texture* t = SafeCast<Texture*>(state.DepthStencil.Get());

		depthView = t->Dsv.GetCpuHandle();
		this->m_trackedResources->Resources.push_back(state.DepthStencil);
		hasDepth = true;
	}

	int numRenderTargets = renderTargetViews.size();
	this->m_internalList->OMSetRenderTargets(
		numRenderTargets,
		renderTargetViews.data(),
		hasDepth,
		hasDepth ? &depthView : nullptr);

	if (state.VertexBuffer)
	{
		Buffer* b = SafeCast<Buffer*>(state.VertexBuffer);
		this->m_internalList->IASetVertexBuffers(0, 1, &b->VertexView);
		this->m_trackedResources->NativeResources.push_back(b->D3DResource);
	}

	if (state.IndexBuffer)
	{
		Buffer* b = SafeCast<Buffer*>(state.IndexBuffer);
		this->m_internalList->IASetIndexBuffer(&b->IndexView);
		this->m_trackedResources->NativeResources.push_back(b->D3DResource);
	}
}

void Dx12Core::Dx12CommandContext::BindScissorRects(std::vector<Rect> const& rects)
{
	this->m_numScissor = rects.size();
	for (int i = 0; i < this->m_numScissor; i++)
	{
		const Rect& scissor = rects[i];

		this->m_dx12Scissor[i] = CD3DX12_RECT(
			scissor.MinX,
			scissor.MinY,
			scissor.MaxX,
			scissor.MaxY);
	}

	this->m_internalList->RSSetScissorRects(this->m_numScissor, this->m_dx12Scissor);
}

void Dx12Core::Dx12CommandContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	this->m_internalList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void Dx12Core::Dx12CommandContext::DrawIndexed(
	uint32_t indexCount,
	uint32_t instanceCount,
	uint32_t startIndex,
	int32_t baseVertex,
	uint32_t startInstance)
{
	this->m_internalList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
}

void Dx12Core::Dx12CommandContext::WriteBuffer(
	IBuffer* buffer,
	const void* data,
	size_t byteSize,
	uint64_t destOffsetBytes)
{
	// TODO: See how the upload buffer can be used here
	RefCountPtr<ID3D12Resource> intermediateResource;
	ThrowIfFailed(
		this->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&intermediateResource)));

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data;
	subresourceData.RowPitch = byteSize;
	subresourceData.SlicePitch = subresourceData.RowPitch;

	Buffer* internal = SafeCast<Buffer*>(buffer);
	UpdateSubresources(
		this->m_internalList.Get(),
		internal->D3DResource,
		intermediateResource,
		0, 0, 1, &subresourceData);

	this->m_trackedResources->NativeResources.push_back(intermediateResource);
}

void Dx12Core::Dx12CommandContext::WriteTexture(
	ITexture* texture,
	uint32_t firstSubresource,
	size_t numSubresources,
	D3D12_SUBRESOURCE_DATA* subresourceData)
{
	Texture* internal = SafeCast<Texture*>(texture);
	UINT64 requiredSize = GetRequiredIntermediateSize(internal->D3DResource, firstSubresource, numSubresources);

	RefCountPtr<ID3D12Resource> intermediateResource;
	ThrowIfFailed(
		this->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&intermediateResource)));

	UpdateSubresources(
		this->m_internalList,
		internal->D3DResource,
		intermediateResource,
		0,
		firstSubresource,
		numSubresources,
		subresourceData);

	this->m_trackedResources->NativeResources.push_back(internal->D3DResource);
	this->m_trackedResources->NativeResources.push_back(intermediateResource);
}

void Dx12Core::Dx12CommandContext::WriteTexture(
	ITexture* texture,
	uint32_t arraySlice,
	uint32_t mipLevel,
	const void* data,
	size_t rowPitch,
	size_t depthPitch)
{
	assert(false);
	Texture* internal = SafeCast<Texture*>(texture);

	uint32_t subresource = 
		CalcSubresource(
			mipLevel,
			arraySlice, 
			0,
			internal->GetDesc().MipLevels,
			internal->GetDesc().ArraySize);

	D3D12_RESOURCE_DESC resourceDesc = internal->D3DResource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	uint32_t numRows;
	uint64_t rowSizeInBytes;
	uint64_t totalBytes;

	this->m_device->GetCopyableFootprints(&resourceDesc, subresource, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

	RefCountPtr<ID3D12Resource> intermediateResource;
	ThrowIfFailed(
		this->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&intermediateResource)));


	assert(numRows <= footprint.Footprint.Height);

	for (uint32_t depthSlice = 0; depthSlice < footprint.Footprint.Depth; depthSlice++)
	{
		for (uint32_t row = 0; row < numRows; row++)
		{
			void* destAddress = (char*)data + footprint.Footprint.RowPitch * (row + depthSlice * numRows);
			const void* srcAddress = (const char*)data + rowPitch * row + depthPitch * depthSlice;
			memcpy(destAddress, srcAddress, std::min(rowPitch, rowSizeInBytes));
		}
	}
}

void Dx12Core::Dx12CommandContext::BindGraphics32BitConstants(
	uint32_t rootParameterIndex,
	uint32_t numConstants,
	const void* constants)
{
	this->m_internalList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void Dx12Core::Dx12CommandContext::BindDynamicConstantBuffer(
	size_t rootParameterIndex,
	size_t sizeInBytes,
	const void* bufferData)
{
	UploadBuffer::Allocation alloc = this->m_uploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	std::memcpy(alloc.Cpu, bufferData, sizeInBytes);

	this->m_internalList->SetGraphicsRootConstantBufferView(rootParameterIndex, alloc.Gpu);
}

void Dx12Core::Dx12CommandContext::BindDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
{
	size_t bufferSize = numVertices * vertexSize;
	auto heapAllocation = this->m_uploadBuffer->Allocate(bufferSize, vertexSize);
	memcpy(heapAllocation.Cpu, vertexBufferData, bufferSize);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = heapAllocation.Gpu;
	vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	vertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);

	this->m_internalList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void Dx12Core::Dx12CommandContext::BindDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData)
{
	size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
	size_t bufferSize = numIndicies * indexSizeInBytes;

	auto heapAllocation = this->m_uploadBuffer->Allocate(bufferSize, indexSizeInBytes);
	memcpy(heapAllocation.Cpu, indexBufferData, bufferSize);

	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = heapAllocation.Gpu;
	indexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
	indexBufferView.Format = indexFormat;

	this->m_internalList->IASetIndexBuffer(&indexBufferView);
}

void Dx12Core::Dx12CommandContext::BindDynamicStructuredBuffer(uint32_t rootParameterIndex, size_t numElements, size_t elementSize, const void* bufferData)
{
	size_t sizeInBytes = numElements * elementSize;
	UploadBuffer::Allocation alloc = this->m_uploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	std::memcpy(alloc.Cpu, bufferData, sizeInBytes);

	this->m_internalList->SetGraphicsRootShaderResourceView(rootParameterIndex, alloc.Gpu);
}

void Dx12Core::Dx12CommandContext::BindStructuredBuffer(size_t rootParameterIndex, IBuffer* buffer)
{
	Buffer* internal = SafeCast<Buffer*>(buffer);

	this->m_internalList->SetGraphicsRootShaderResourceView(rootParameterIndex, internal->D3DResource->GetGPUVirtualAddress());
}

void Dx12Core::Dx12CommandContext::BindBindlessDescriptorTables(size_t rootParamterIndex)
{
	if (m_cbvSrvUavBindlessTable.ptr != 0)
	{
		this->m_internalList->SetGraphicsRootDescriptorTable(rootParamterIndex, this->m_cbvSrvUavBindlessTable);
	}
}

ScopedMarker Dx12Core::Dx12CommandContext::BeginScropedMarker(std::string name)
{
	this->BeginMarker(name);
	return ScopedMarker(this);
}

void Dx12Core::Dx12CommandContext::BeginMarker(std::string name)
{
	PIXBeginEvent(this->m_internalList, 0, std::wstring(name.begin(), name.end()).c_str());
}

void Dx12Core::Dx12CommandContext::EndMarker()
{
	PIXEndEvent(this->m_internalList);
}
