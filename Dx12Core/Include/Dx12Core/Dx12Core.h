#pragma once

#include <stdint.h>

#include "Dx12Common.h"

#include "RefCountPtr.h"

#include "ResourceId.h"

#include <optional>

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

namespace Dx12Core
{
    struct Color
    {
        float R;
        float G;
        float B;
        float A;

        Color()
            : R(0.f), G(0.f), B(0.f), A(0.f)
        { }

        Color(float c)
            : R(c), G(c), B(c), A(c)
        { }

        Color(float r, float g, float b, float a)
            : R(r), G(g), B(b), A(a) { }

        bool operator ==(const Color& other) const { return R == other.R && G == other.G && B == other.B && A == other.A; }
        bool operator !=(const Color& other) const { return !(*this == other); }
    };

    struct Viewport
    {
        float MinX, MaxX;
        float MinY, MaxY;
        float MinZ, MaxZ;

        Viewport() : MinX(0.f), MaxX(0.f), MinY(0.f), MaxY(0.f), MinZ(0.f), MaxZ(1.f) { }

        Viewport(float width, float height) : MinX(0.f), MaxX(width), MinY(0.f), MaxY(height), MinZ(0.f), MaxZ(1.f) { }

        Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
            : MinX(_minX), MaxX(_maxX), MinY(_minY), MaxY(_maxY), MinZ(_minZ), MaxZ(_maxZ)
        { }

        bool operator ==(const Viewport& b) const
        {
            return MinX == b.MinX
                && MinY == b.MinY
                && MinZ == b.MinZ
                && MaxX == b.MaxX
                && MaxY == b.MaxY
                && MaxZ == b.MaxZ;
        }
        bool operator !=(const Viewport& b) const { return !(*this == b); }

        float GetWidth() const { return MaxX - MinX; }
        float GetHeight() const { return MaxY - MinY; }
    };

    struct Rect
    {
        int MinX, MaxX;
        int MinY, MaxY;

        Rect() : MinX(0), MaxX(0), MinY(0), MaxY(0) { }
        Rect(int width, int height) : MinX(0), MaxX(width), MinY(0), MaxY(height) { }
        Rect(int _minX, int _maxX, int _minY, int _maxY) : MinX(_minX), MaxX(_maxX), MinY(_minY), MaxY(_maxY) { }
        explicit Rect(const Viewport& viewport)
            : MinX(int(floorf(viewport.MinX)))
            , MaxX(int(ceilf(viewport.MaxX)))
            , MinY(int(floorf(viewport.MinY)))
            , MaxY(int(ceilf(viewport.MaxY)))
        {
        }

        bool operator ==(const Rect& b) const {
            return MinX == b.MinX && MinY == b.MinY && MaxX == b.MaxX && MaxY == b.MaxY;
        }
        bool operator !=(const Rect& b) const { return !(*this == b); }

        int GetWidth() const { return MaxX - MinX; }
        int GetHeight() const { return MaxY - MinY; }
    };

    class IResource
    {
    protected:
        IResource() = default;
        virtual ~IResource() = default;

    public:
        virtual unsigned long AddRef() = 0;
        virtual unsigned long Release() = 0;

        // Non-copyable and non-movable
        IResource(const IResource&) = delete;
        IResource(const IResource&&) = delete;
        IResource& operator=(const IResource&) = delete;
        IResource& operator=(const IResource&&) = delete;
    };

    typedef RefCountPtr<IResource> ResourceHandle;


    enum class BindFlags : uint32_t
    {
        None            = 0,
        VertexBuffer    = 0x00000001,
        IndexBuffer     = 0x00000002,
        ConstantBuffer  = 0x00000004,
        ShaderResource  = 0x00000008,
        RenderTarget    = 0x00000010,
        DepthStencil    = 0x00000020,
        UnorderedAccess = 0x00000040,
    };

    ENUM_CLASS_FLAG_OPERATORS(BindFlags)

    enum class TextureDimension : uint8_t
    {
        Unknown,
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        TextureCube,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
        Texture3D
    };

    struct TextureDesc
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
        union
        {
            uint16_t ArraySize = 1;
            uint16_t Depth;
        };

        uint16_t MipLevels = 0;

        TextureDimension Dimension = TextureDimension::Unknown;
        BindFlags Bindings = BindFlags::None;

        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
        std::optional<D3D12_CLEAR_VALUE> OptmizedClearValue;
        D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;
        std::string DebugName;
    };

    class ITexture : public IResource
    {
    public:
        virtual ~ITexture() = default;

        virtual const TextureDesc& GetDesc() const = 0;

    };

    typedef RefCountPtr<ITexture> TextureHandle;

    struct BufferDesc
    {
        std::wstring DebugName = L"";
        uint64_t StrideInBytes = 0;
        uint64_t SizeInBytes = 0;
        BindFlags BindFlags = BindFlags::None;
    };

    class IBuffer : public IResource
    {
    public:
        virtual ~IBuffer() = default;

        virtual const BufferDesc& GetDesc() const = 0;
    };

    typedef RefCountPtr<IBuffer> BufferHandle;

    // Shader type mask. The values match ones used in Vulkan.
    enum class ShaderType : uint16_t
    {
        None = 0x0000,

        Compute = 0x0020,

        Vertex = 0x0001,
        Hull = 0x0002,
        Domain = 0x0004,
        Geometry = 0x0008,
        Pixel = 0x0010,
        Amplification = 0x0040,
        Mesh = 0x0080,
        AllGraphics = 0x00FE,

        RayGeneration = 0x0100,
        AnyHit = 0x0200,
        ClosestHit = 0x0400,
        Miss = 0x0800,
        Intersection = 0x1000,
        Callable = 0x2000,
        AllRayTracing = 0x3F00,

        All = 0x3FFF,
    };

    ENUM_CLASS_FLAG_OPERATORS(ShaderType)

    struct ShaderDesc
    {
        ShaderType shaderType = ShaderType::None;
        std::string debugName = "";
    };

    class IShader : public IResource
    {
    public:
        virtual ~IShader() = default;

        virtual const ShaderDesc& GetDesc() const = 0;
        virtual const std::vector<uint8_t>& GetByteCode() const = 0;
    };

    typedef RefCountPtr<IShader> ShaderHandle;

    struct BindlessShaderParameter
    {
        D3D12_DESCRIPTOR_RANGE_TYPE Type;
        uint32_t BaseShaderRegister = 0;
        uint32_t RegisterSpace = 0;

        BindlessShaderParameter(
            D3D12_DESCRIPTOR_RANGE_TYPE type,
            uint32_t registerSpace)
            : Type(type)
            , RegisterSpace(registerSpace)
        {
        }
    };

    struct ShaderResourceVariable
    {
        ShaderType ShaderStages = ShaderType::All;
        std::string Name = "";
        uint32_t ShaderRegister = 0;
        uint32_t ShaderSpace = 0;

        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;

        bool IsBindless = false;
        uint32_t MaxSize = 1;

        bool IsPushConstant = false;
        uint32_t Num32BitConstants = 0;

        BindFlags Binding = BindFlags::None;

        static ShaderResourceVariable CreateTextureSrv(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);
            retVal.Binding = BindFlags::ShaderResource;
            return retVal;
        }

        static ShaderResourceVariable CreateBindlessSrv(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace,
            uint32_t maxSize)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);
            retVal.Binding = BindFlags::ShaderResource;
            retVal.IsBindless = true;
            retVal.MaxSize = maxSize; // maybe not?
            return retVal;
        }

        static ShaderResourceVariable CreatePushConstant(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);

            return retVal;
        }

        static ShaderResourceVariable CreateSrv(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);

            return retVal;
        }

        static ShaderResourceVariable CreateCbv(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);

            return retVal;
        }

        static ShaderResourceVariable CreateStaticSampler(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace,
            D3D12_FILTER			   filter,
            D3D12_TEXTURE_ADDRESS_MODE addressUVW,
            UINT					   maxAnisotropy = 16,
            D3D12_COMPARISON_FUNC	   comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR  borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
        {
            auto retVal = Create(shaderStages, name, shaderRegister, shaderSpace);

            return retVal;
        }

    private:
        static ShaderResourceVariable Create(
            ShaderType shaderStages,
            std::string const& name,
            uint32_t shaderRegister,
            uint32_t shaderSpace)
        {
            ShaderResourceVariable retVal = {};
            retVal.ShaderRegister = shaderRegister;
            retVal.Name = name;
            retVal.ShaderRegister = shaderRegister;
            retVal.ShaderSpace = shaderSpace;

            return retVal;
        }
    };

    struct BindlessShaderParameterLayout
    {
        D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL;
        uint32_t FirstSlot = 0;
        uint32_t MaxCapacity = UINT_MAX;
        std::vector<BindlessShaderParameter> Parameters;

        BindlessShaderParameterLayout& AddParameterSRV(uint32_t registerSpace)
        {
            return this->AddParameter(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                registerSpace);
        }

        BindlessShaderParameterLayout& AddParameterUAV(uint32_t registerSpace)
        {
            return this->AddParameter(
                D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                registerSpace);
        }

        BindlessShaderParameterLayout& AddParameterCBV(uint32_t registerSpace)
        {
            return this->AddParameter(
                D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                registerSpace);
        }

        BindlessShaderParameterLayout& AddParameter(
            D3D12_DESCRIPTOR_RANGE_TYPE type,
            uint32_t registerSpace)
        {
            this->Parameters.emplace_back(
                BindlessShaderParameter(type, registerSpace));

            return *this;
        }
    };

    struct ShaderParameterLayout
    {
        std::vector<CD3DX12_ROOT_PARAMETER1> Parameters;
        std::vector<CD3DX12_STATIC_SAMPLER_DESC> StaticSamplers;

        template<UINT ShaderRegister, UINT RegisterSpace>
        ShaderParameterLayout& AddConstantParameter(UINT num32BitValues)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsConstants(num32BitValues, ShaderRegister, RegisterSpace);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        ShaderParameterLayout& AddCBVParameter(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsConstantBufferView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        ShaderParameterLayout& AddSRVParameter(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsShaderResourceView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        ShaderParameterLayout& AddUAVParameter(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsUnorderedAccessView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        ShaderParameterLayout& AddStaticSampler(
            D3D12_FILTER			   filter,
            D3D12_TEXTURE_ADDRESS_MODE addressUVW,
            UINT					   maxAnisotropy = 16,
            D3D12_COMPARISON_FUNC	   comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR  borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
        {
            CD3DX12_STATIC_SAMPLER_DESC& desc = this->StaticSamplers.emplace_back();
            desc.Init(
                ShaderRegister,
                filter,
                addressUVW,
                addressUVW,
                addressUVW,
                0.0f,
                maxAnisotropy,
                comparisonFunc,
                borderColor);
            desc.RegisterSpace = RegisterSpace;

            return *this;
        }

        void AddParameter(D3D12_ROOT_PARAMETER1 parameter)
        {
            Parameters.emplace_back(parameter);
        }
    };

	struct DescriptorTableDesc
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> DescriptorRanges;

		template<UINT BaseShaderRegister, UINT RegisterSpace>
		DescriptorTableDesc& AddSRVRange(
			UINT numDescriptors,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags,
			UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			return this->AddDescriptorRange<BaseShaderRegister, RegisterSpace>(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				numDescriptors,
				flags,
				offsetInDescriptorsFromTableStart);
		}

		template<UINT BaseShaderRegister, UINT RegisterSpace>
		DescriptorTableDesc& AddUAVRange(
			UINT numDescriptors,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags,
			UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			return this->AddDescriptorRange<BaseShaderRegister, RegisterSpace>(
				D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
				numDescriptors,
				flags,
				offsetInDescriptorsFromTableStart);
		}

		template<UINT BaseShaderRegister, UINT RegisterSpace>
		DescriptorTableDesc& AddCBVRange(
			UINT numDescriptors,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags,
			UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			return this->AddDescriptorRange<BaseShaderRegister, RegisterSpace>(
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				numDescriptors,
				flags,
				offsetInDescriptorsFromTableStart);
		}

		template<UINT BaseShaderRegister, UINT RegisterSpace>
		DescriptorTableDesc& AddSamplerRange(
			UINT numDescriptors,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags,
			UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			return this->AddDescriptorRange<BaseShaderRegister, RegisterSpace>(
				D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
				numDescriptors,
				flags,
				offsetInDescriptorsFromTableStart);
		}

		template<UINT BaseShaderRegister, UINT RegisterSpace>
		DescriptorTableDesc& AddDescriptorRange(
			D3D12_DESCRIPTOR_RANGE_TYPE type,
			UINT numDescriptors,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags,
			UINT offsetInDescriptorsFromTableStart)
		{
			CD3DX12_DESCRIPTOR_RANGE1& range = this->DescriptorRanges.emplace_back();
			range.Init(type, numDescriptors, BaseShaderRegister, RegisterSpace, flags, offsetInDescriptorsFromTableStart);

			return *this;
		}

		operator D3D12_ROOT_DESCRIPTOR_TABLE1() const
		{
			return D3D12_ROOT_DESCRIPTOR_TABLE1{ static_cast<UINT>(this->DescriptorRanges.size()),
												 this->DescriptorRanges.data() };
		}

		const auto& GetDescriptorRanges() const noexcept { return this->DescriptorRanges; }
		UINT Size() const noexcept { return static_cast<UINT>(this->DescriptorRanges.size()); }
	};

    struct RootSignatureDesc
    {
        std::vector<CD3DX12_ROOT_PARAMETER1> Parameters;
        std::vector<CD3DX12_STATIC_SAMPLER_DESC> StaticSamplers;

        std::vector<UINT> DescriptorTableIndices;
        std::vector<DescriptorTableDesc> DescriptorTables;

        D3D12_ROOT_SIGNATURE_FLAGS Flags;

        RootSignatureDesc& AddDescriptorTable(const DescriptorTableDesc& descriptorTable)
        {
            CD3DX12_ROOT_PARAMETER1& parameter = Parameters.emplace_back();
            parameter.InitAsDescriptorTable(descriptorTable.Size(), descriptorTable.GetDescriptorRanges().data());

            // The descriptor table descriptor ranges require a pointer to the descriptor ranges. Since new
            // ranges can be dynamically added in the vector, we separately store the index of the range set.
            // The actual address will be solved when generating the actual root signature
            this->DescriptorTableIndices.push_back(static_cast<UINT>(this->DescriptorTables.size()));
            this->DescriptorTables.push_back(descriptorTable);

            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        RootSignatureDesc& Add32BitConstants(UINT num32BitValues)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsConstants(num32BitValues, ShaderRegister, RegisterSpace);

            this->AddParameter(parameter);
            return *this;
        }

        RootSignatureDesc& Add32BitConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsConstants(num32BitValues, shaderRegister, registerSpace);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        RootSignatureDesc& AddConstantBufferView(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsConstantBufferView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        RootSignatureDesc& AddShaderResourceView(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsShaderResourceView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        RootSignatureDesc& AddUnorderedAccessView(D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
        {
            CD3DX12_ROOT_PARAMETER1 parameter = {};
            parameter.InitAsUnorderedAccessView(ShaderRegister, RegisterSpace, flags);

            this->AddParameter(parameter);
            return *this;
        }

        template<UINT ShaderRegister, UINT RegisterSpace>
        RootSignatureDesc& AddStaticSampler(
            D3D12_FILTER			   filter,
            D3D12_TEXTURE_ADDRESS_MODE addressUVW,
            UINT					   maxAnisotropy,
            D3D12_COMPARISON_FUNC	   comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR  borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
        {
            CD3DX12_STATIC_SAMPLER_DESC& desc = this->StaticSamplers.emplace_back();
            desc.Init(
                ShaderRegister,
                filter,
                addressUVW,
                addressUVW,
                addressUVW,
                0.0f,
                maxAnisotropy,
                comparisonFunc,
                borderColor);
            desc.RegisterSpace = RegisterSpace;
            return *this;
        }

        void AddParameter(D3D12_ROOT_PARAMETER1 Parameter)
        {
            Parameters.emplace_back(Parameter);
            // HUH?
            DescriptorTableIndices.emplace_back(0xDEADBEEF);
        }

        RootSignatureDesc& AllowInputLayout() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; return *this; }
        RootSignatureDesc& DenyVSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; return *this; };
        RootSignatureDesc& DenyHSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; return *this; };
        RootSignatureDesc& DenyDSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; return *this; };
        RootSignatureDesc& DenyTessellationShaderAccess() noexcept
        {
            this->DenyHSAccess();
            this->DenyDSAccess(); 
            return *this;
        };
        RootSignatureDesc& DenyGSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; return *this; }
        RootSignatureDesc& DenyPSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; return *this; }
        RootSignatureDesc& AllowStreamOutput() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT; return *this; }
        RootSignatureDesc& SetAsLocalRootSignature() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE; return *this; }
        RootSignatureDesc& DenyASAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; return *this; }
        RootSignatureDesc& DenyMSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; return *this; }
		RootSignatureDesc& AllowResourceDescriptorHeapIndexing() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED; return *this; }
		RootSignatureDesc& AllowSampleDescriptorHeapIndexing() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED; return *this; }

        D3D12_ROOT_SIGNATURE_DESC1 BuildDx12Desc() noexcept
        {
            // Go through all the parameters, and set the actual addresses of the heap range descriptors based
            // on their indices in the range indices vector
            for (size_t i = 0; i < this->Parameters.size(); ++i)
            {
                if (this->Parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                {
                    this->Parameters[i].DescriptorTable = this->DescriptorTables[DescriptorTableIndices[i]];
                }
            }

            D3D12_ROOT_SIGNATURE_DESC1 desc = {};
            desc.NumParameters = static_cast<UINT>(this->Parameters.size());
            desc.pParameters = this->Parameters.data();
            desc.NumStaticSamplers = static_cast<UINT>(this->StaticSamplers.size());
            desc.pStaticSamplers = this->StaticSamplers.data();
            desc.Flags = this->Flags;

            return desc;
        }
	};

    struct RenderState
    {
        std::vector<DXGI_FORMAT> RtvFormats;
        DXGI_FORMAT DsvFormat;
        CD3DX12_DEPTH_STENCIL_DESC* DepthStencilState = nullptr;
        CD3DX12_BLEND_DESC* BlendState = nullptr;
        CD3DX12_RASTERIZER_DESC* RasterizerState = nullptr;
    };

    struct GraphicsPipelineDesc
    {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
        
        RootSignatureDesc RootSignatureDesc = {};

        bool UseShaderParameters = false;

        struct ShaderParameters
        {
            D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
            ShaderParameterLayout* Binding = nullptr;
            BindlessShaderParameterLayout* Bindless = nullptr;

            void AllowInputLayout() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; }
            void DenyVSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; };
            void DenyHSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; };
            void DenyDSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; };
            void DenyTessellationShaderAccess() noexcept
            {
                this->DenyHSAccess();
                this->DenyDSAccess();
            };
            void DenyGSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; }
            void DenyPSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; }
            void AllowStreamOutput() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT; }
            void SetAsLocalRootSignature() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE; }
            void DenyASAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; }
            void DenyMSAccess() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; }
            void AllowResourceDescriptorHeapIndexing() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED; }
            void AllowSampleDescriptorHeapIndexing() noexcept { this->Flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED; }
        };

        ShaderParameters ShaderParameters = {};

        ShaderHandle VS;
        ShaderHandle HS;
        ShaderHandle DS;
        ShaderHandle GS;
        ShaderHandle PS;

        RenderState RenderState = {};
    };

    class IGraphicsPipeline : public IResource
    {
    public:
        virtual ~IGraphicsPipeline() = default;

        virtual const GraphicsPipelineDesc& GetDesc() const = 0;
    };

    typedef RefCountPtr<IGraphicsPipeline> GraphicsPipelineHandle;

    struct GraphicsDeviceDesc
    {
        bool EnableComputeQueue = false;
        bool EnableCopyQueue = false;

        uint32_t RenderTargetViewHeapSize = 1024;
        uint32_t DepthStencilViewHeapSize = 1024;
        uint32_t ShaderResourceViewCpuHeapSize = 1024;
        uint32_t ShaderResourceViewGpuStaticHeapSize = 16384;
        uint32_t ShaderResourceViewGpuDynamicHeapSize = 16384;
        uint32_t SamplerHeapCpuSize = 1024;
        uint32_t SamplerHeapGpuSize = 1024;

    };

    struct SwapChainDesc
    {
        uint32_t NumBuffers = 3;
        uint32_t Width = 0;
        uint32_t Height = 0;
        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
        HWND WindowHandle = nullptr;
    };

    struct GraphicsState
    {
        IGraphicsPipeline* PipelineState = nullptr;
        IBuffer* VertexBuffer = nullptr;
        IBuffer* IndexBuffer = nullptr;
        std::vector<Viewport> Viewports;
        std::vector<Rect> ScissorRect; // TODO: consider arrays
        std::vector<TextureHandle> RenderTargets;
        TextureHandle DepthStencil = nullptr;
    };

    class ScopedMarker;

    class ICommandContext : public IResource
    {
    public:
        virtual ~ICommandContext() = default;

        virtual void TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) = 0;
        virtual void TransitionBarrier(IBuffer* buffer, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) = 0;

        virtual void ClearTextureFloat(ITexture* texture, Color const& clearColour) = 0;
        virtual void ClearDepthStencilTexture(ITexture* depthStencil, bool clearDepth, float depth, bool clearStencil, uint8_t stencil) = 0;

        virtual void SetGraphicsState(GraphicsState& state) = 0;
        virtual void BindScissorRects(std::vector<Rect> const& rects) = 0;

        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0) = 0;
        virtual void DrawIndexed(
            uint32_t indexCount,
            uint32_t instanceCount = 1,
            uint32_t startIndex = 0,
            int32_t baseVertex = 0,
            uint32_t startInstance = 0) = 0;

        template<typename T>
        void WriteBuffer(IBuffer* buffer, std::vector<T> data, uint64_t destOffsetBytes = 0)
        {
            this->WriteBuffer(buffer, data.data(), sizeof(T) * data.size(), destOffsetBytes);
        }

        virtual void WriteBuffer(IBuffer* buffer, const void* data, size_t dataSize, uint64_t destOffsetBytes = 0) = 0;
        virtual void WriteTexture(
            ITexture* texture,
            uint32_t firstSubResource,
            size_t numSubResources,
            D3D12_SUBRESOURCE_DATA* subresourceData) = 0;

        virtual void WriteTexture(
            ITexture* texture,
            uint32_t arraySlize,
            uint32_t mipLevel,
            const void* data,
            size_t rowPitch,
            size_t depthPitch) = 0;

        virtual void BindGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants) = 0;
        template<typename T>
        void BindGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants)
        {
            static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
            this->BindGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
        }

        virtual void BindDynamicConstantBuffer(size_t rootParameterIndex, size_t sizeInBytes, const void* bufferData) = 0;
        template<typename T>
        void BindDynamicConstantBuffer(size_t rootParameterIndex, T const& bufferData)
        {
            this->BindDynamicConstantBuffer(rootParameterIndex, sizeof(T), &bufferData);
        }

        /**
         * Set dynamic vertex buffer data to the rendering pipeline.
         */
        virtual void BindDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData) = 0;

        template<typename T>
        void BindDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
        {
            this->SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
        }

        /**
         * Bind dynamic index buffer data to the rendering pipeline.
         */
        virtual void BindDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData) = 0;

        template<typename T>
        void BindDynamicIndexBuffer(const std::vector<T>& indexBufferData)
        {
            static_assert(sizeof(T) == 2 || sizeof(T) == 4);

            DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
            this->SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
        }

        /**
         * Set dynamic structured buffer contents.
         */
        virtual void BindDynamicStructuredBuffer(uint32_t rootParameterIndex, size_t numElements, size_t elementSize, const void* bufferData) = 0;

        template<typename T>
        void BindDynamicStructuredBuffer(uint32_t rootParameterIndex, const std::vector<T>& bufferData)
        {
            this->BindDynamicStructuredBuffer(rootParameterIndex, bufferData.size(), sizeof(T), bufferData.data());
        }

        virtual void BindStructuredBuffer(size_t rootParameterIndex, IBuffer* buffer) = 0;

        // Depericated
        virtual void BindBindlessDescriptorTables(size_t rootParamterIndex) = 0;

        virtual ScopedMarker BeginScropedMarker(std::string name) = 0;
        virtual void BeginMarker(std::string name) = 0;
        virtual void EndMarker() = 0;

    };

    typedef RefCountPtr<ICommandContext> CommandContextHandle;

    class ScopedMarker
    {
    public:
        ScopedMarker(ICommandContext* context)
            : m_context(context) {}

        ~ScopedMarker() { this->m_context->EndMarker(); }

    private:
        ICommandContext* m_context;
    };

    class IGraphicsDevice : public IResource
    {
    public:
        virtual ~IGraphicsDevice() = default;

        virtual void InitializeSwapcChain(SwapChainDesc const& swapChainDesc) = 0;

        virtual void BeginFrame() = 0;
        virtual void Present() = 0;

        virtual ICommandContext& BeginContext() = 0;
        virtual uint64_t Submit(bool waitForCompletion = false) = 0;

        virtual void WaitForIdle() const = 0;

    public:
        virtual DescriptorIndex GetDescritporIndex(ITexture* texture) const = 0;
        virtual DescriptorIndex GetDescritporIndex(IBuffer* buffer) const = 0;

    public:
        virtual TextureHandle CreateTexture(TextureDesc desc) = 0;
        virtual TextureHandle CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native) = 0;

        virtual BufferHandle CreateBuffer(BufferDesc desc) = 0;

        virtual ShaderHandle CreateShader(ShaderDesc const& desc, const void* binary, size_t binarySize) = 0;

        virtual GraphicsPipelineHandle CreateGraphicPipeline(GraphicsPipelineDesc desc) = 0;

    public:
        virtual const GraphicsDeviceDesc& GetDesc() const = 0;
        virtual const SwapChainDesc& GetCurrentSwapChainDesc() const = 0;

        virtual TextureHandle GetCurrentBackBuffer() = 0;
        virtual TextureHandle GetBackBuffer(uint32_t index) = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;

    };

    // TODO - make this more clear
    typedef RefCountPtr<IGraphicsDevice> GraphicsDeviceHandle;

}