#pragma once

#include <stdint.h>

#include "Dx12Common.h"

#include "RefCountPtr.h"

#include "ResourceId.h"

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

    class ICommandContext : public IResource
    {
    public:
        virtual ~ICommandContext() = default;

        virtual void TransitionBarrier(ITexture* texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) = 0;

        virtual void ClearTextureFloat(ITexture* texture, Color const& clearColour) = 0;

        virtual void BeginMarker(std::string name) = 0;
        virtual void EndMarker() = 0;

    };
    typedef RefCountPtr<ICommandContext> CommandContextHandle;

    class IGraphicsDevice : public IResource
    {
    public:
        virtual ~IGraphicsDevice() = default;

        virtual void InitializeSwapcChain(SwapChainDesc const& swapChainDesc) = 0;

        virtual void BeginFrame() = 0;
        virtual void Present() = 0;

        virtual ICommandContext& BeginContext() = 0;
        virtual uint64_t Submit() = 0;

        virtual void WaitForIdle() const = 0;

    public:
        virtual TextureHandle CreateTextureFromNative(TextureDesc desc, RefCountPtr<ID3D12Resource> native) = 0;

    public:
        virtual const GraphicsDeviceDesc& GetDesc() const = 0;
        virtual const SwapChainDesc& GetCurrentSwapChainDesc() const = 0;

        virtual ITexture* GetCurrentBackBuffer() = 0;
        virtual ITexture* GetBackBuffer(uint32_t index) = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;

    };

    // TODO - make this more clear
    typedef RefCountPtr<IGraphicsDevice> GraphicsDeviceHandle;

}