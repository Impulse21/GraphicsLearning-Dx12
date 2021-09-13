#pragma once

#include <pix.h>
#include <memory>

#include "RefCountPtr.h"
#include "Dx12Core.h"
#include "Dx12Queue.h"

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) =override; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

namespace Dx12Core
{
    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,
        Copy,

        Count
    };

    class D3D12Resource
    {
    public:
        ID3D12Resource* GetNative() const { return this->m_d3dResource; }

    protected:
        RefCountPtr<ID3D12Resource> m_d3dResource;
    };

    class GraphicsDevice;
    class Texture : public RefCounter<ITexture>, public D3D12Resource
    {
    public:
        Texture(
            Dx12Context const& context,
            DeviceResources const& resources,
            TextureDesc desc,
            RefCountPtr<ID3D12Resource> nativeResource);

        const TextureDesc& GetDesc() const override { return this->m_desc; }

        D3D12_CPU_DESCRIPTOR_HANDLE GetRtv();

        void CreateViews();

    private:
        const Dx12Context& m_context;
        const DeviceResources& m_deviceResources;
        const TextureDesc m_desc;


        DescriptorIndex m_rtv = INVALID_DESCRIPTOR_INDEX;

    };

    class CommandContext : public RefCounter<ICommandContext>
    {
    public:
        CommandContext(GraphicsDevice* device);
        ~CommandContext() = default;

        void Begin() override;
        void Close() override;

        void TransitionBarrier(ITexture * texture, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) override;

        void ClearRenderTarget(ITexture * rexture, Color const& color) override;

        void BeginMarker(std::string name) override;
        void EndMarker() override;


    public:
        ID3D12GraphicsCommandList* GetNative() { return this->m_internalCommandList; }
        
        void Executed(uint64_t fenceValue);

    private:
        GraphicsDevice* m_device;

        RefCountPtr<ID3D12GraphicsCommandList> m_internalCommandList;
        ID3D12CommandAllocator* m_activeAllocator;
        CommandAllocatorPool m_allocatorPool;
    };
}