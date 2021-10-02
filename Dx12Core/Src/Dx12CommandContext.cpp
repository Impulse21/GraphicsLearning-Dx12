#include "Dx12CommandContext.h"

#include <pix.h>

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
}

void Dx12Core::Dx12CommandContext::Reset(uint64_t completedFenceValue)
{
	assert(this->m_allocator == nullptr);

	this->m_allocator = this->m_allocatorPool.RequestAllocator(completedFenceValue);

	this->m_internalList->Reset(this->m_allocator, nullptr);

	this->m_trackedResources = std::make_shared<ReferencedResources>();
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

void Dx12Core::Dx12CommandContext::SetGraphicsState(GraphicsState& state)
{
	GraphicsPipeline* pipeline = SafeCast<GraphicsPipeline*>(state.PipelineState);
	this->m_internalList->SetPipelineState(pipeline->D3DPipelineState);

	// Root Signature without casting? Maybe keep a week reference in the Graphics pipline state object;
	this->m_internalList->SetGraphicsRootSignature(pipeline->D3DRootSignature);

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

	this->m_numScissor = state.ScissorRect.size();
	for (int i = 0; i < this->m_numScissor; i++)
	{
		Rect& scissor = state.ScissorRect[i];

		this->m_dx12Scissor[i] = CD3DX12_RECT(
			scissor.MinX,
			scissor.MinY,
			scissor.MaxX,
			scissor.MaxY);
	}
	
	this->m_internalList->RSSetScissorRects(this->m_numScissor, this->m_dx12Scissor);

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	for (auto renderTarget : state.RenderTargets)
	{
		Texture* t = SafeCast<Texture*>(renderTarget.Get());

		renderTargetViews.push_back(t->Rtv.GetCpuHandle());
		this->m_trackedResources->Resources.push_back(renderTarget);
	}

	if (!renderTargetViews.empty())
	{
		this->m_internalList->OMSetRenderTargets(renderTargetViews.size(), renderTargetViews.data(), false, nullptr);
	}

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
