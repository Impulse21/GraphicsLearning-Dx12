#pragma once

#include "RefCountPtr.h"

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

        virtual void InitializeSwapcChain(SwapChainDesc swapChainDesc) = 0;

        virtual void BeginFrame() = 0;
        virtual void Present() = 0;

    public:
        virtual const GraphicsDeviceDesc& GetDesc() const = 0;
        virtual const SwapChainDesc& GetCurrentSwapChainDesc() const = 0;

        virtual uint32_t GetCurrentBackBuffer() = 0;
        virtual uint32_t GetBackBuffer(uint32_t index) = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;
    };

    typedef RefCountPtr<IGraphicsDevice> GraphicsDeviceHandle;

}