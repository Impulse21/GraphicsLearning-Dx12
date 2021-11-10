#pragma once

#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Dx12Common.h"

#include "DescriptorAllocation.h"

namespace Dx12Core
{
	class GraphicsDevice;


	class Shader : public RefCounter<IShader>
	{
	public:
		Shader(ShaderDesc const& desc, const void * binary, size_t binarySize)
			: m_desc(desc)
		{
			this->m_byteCode.resize(binarySize);
			std::memcpy(this->m_byteCode.data(), binary, binarySize);
		}

		const ShaderDesc& GetDesc() const { return this->m_desc; }
		const std::vector<uint8_t>& GetByteCode() const { return this->m_byteCode; }


	private:
		std::vector<uint8_t> m_byteCode;
		const ShaderDesc m_desc;
	};

	class RootSignature : public RefCounter<IResource>
	{
	public:
		explicit RootSignature(RefCountPtr<ID3D12RootSignature> rootSignature)
			: D3DRootSignature(rootSignature) {}

		RefCountPtr<ID3D12RootSignature> D3DRootSignature;

		uint32_t BindlessRootParameterOffset = 0;
	};

	typedef RefCountPtr<RootSignature> RootSignatureHandle;

	class GraphicsPipeline : public RefCounter<IGraphicsPipeline>
	{
	public:
		GraphicsPipeline(GraphicsPipelineDesc&& desc)
			: m_desc(std::move(desc))
		{}

		RefCountPtr<ID3D12PipelineState> D3DPipelineState;
		RootSignatureHandle RootSignature;

		const GraphicsPipelineDesc& GetDesc() const override { return this->m_desc; }

		bool HasBindlessParamaters = false;
		D3D12_GPU_DESCRIPTOR_HANDLE bindlessResourceTable;
		// TODO: D3D12_GPU_DESCRIPTOR_HANDLE bindlessSamplerTable;

	private:
		const GraphicsPipelineDesc m_desc;
	};

	class Buffer : public RefCounter<IBuffer>
	{
	public:
		Buffer(BufferDesc&& desc)
			: m_desc(desc) 
		{};

		const BufferDesc& GetDesc() const override { return this->m_desc; }

		RefCountPtr<ID3D12Resource> D3DResource;

		DescriptorAllocation Srv = {};

		D3D12_VERTEX_BUFFER_VIEW VertexView = {};
		D3D12_INDEX_BUFFER_VIEW IndexView = {};

	private:
		BufferDesc m_desc;
	};

	class Texture : public RefCounter<ITexture>
	{
	public:
		Texture(GraphicsDevice* device, TextureDesc const& desc)
			: m_desc(std::move(desc))
			, Device(device)
		{
		}

		~Texture() = default;

		RefCountPtr<ID3D12Resource> D3DResource = nullptr;
		DescriptorAllocation Rtv = {};
		DescriptorAllocation Dsv = {};
		DescriptorAllocation Srv = {};

		GraphicsDevice* Device;

		const TextureDesc& GetDesc() const override { return this->m_desc; }

	private:
		TextureDesc m_desc;
	};
}
