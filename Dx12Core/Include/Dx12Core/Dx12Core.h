#pragma once

#include <stdint.h>

#include "Dx12Common.h"

#include "RefCountPtr.h"

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

namespace Dx12Core
{
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
        None = 0,
        VertexBuffer = 0x00000001,
        IndexBuffer = 0x00000002,
        ConstantBuffer = 0x00000004,
        ShaderResource = 0x00000008,
        RenderTarget = 0x00000010,
        DepthStencil = 0x00000020,
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
        uint32_t Width;
        uint32_t Height;

        TextureDimension Dimension = TextureDimension::Unknown;
        BindFlags Bindings = BindFlags::None;

        D3D12_RESOURCE_STATES ResourceState;

        DXGI_FORMAT Format;

        std::string DebugName;
    };

    class ITexture : public IResource
    {
    public:
        virtual ~ITexture() = default;

        virtual const TextureDesc& GetDesc() const = 0;

    };

    typedef RefCountPtr<ITexture> TextureHandle;

    struct GraphicsDeviceDesc
    {
        bool EnableComputeQueue = false;
        bool EnableCopyQueue = false;

        uint32_t RenderTargetViewHeapSize = 1024;
        uint32_t DepthStencilViewHeapSize = 1024;
        uint32_t ShaderResourceViewHeapSize = 16384;
        uint32_t SamplerHeapSize = 1024;
    };

    struct SwapChainDesc
    {
        uint32_t NumBuffers = 3;
        uint32_t Width = 0;
        uint32_t Height = 0;
        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
        HWND WindowHandle = nullptr;
    };

    class IGraphicsDevice : public IResource
    {
    public:
        virtual ~IGraphicsDevice() = default;

        virtual void InitializeSwapcChain(SwapChainDesc const& swapChainDesc) = 0;

        virtual void BeginFrame() = 0;
        virtual void Present() = 0;

        virtual void WaitForIdle() const = 0;

    public:
        virtual TextureHandle CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native) = 0;

    public:
        virtual const GraphicsDeviceDesc& GetDesc() const = 0;
        virtual const SwapChainDesc& GetCurrentSwapChainDesc() const = 0;

        virtual uint32_t GetCurrentBackBuffer() = 0;
        virtual uint32_t GetBackBuffer(uint32_t index) = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;
    };

    typedef RefCountPtr<IGraphicsDevice> GraphicsDeviceHandle;

}