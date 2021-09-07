#pragma once

#include "RefCountPtr.h"
#include "Dx12Core.h"

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

namespace Dx12Core
{

    class Texture : public RefCounter<ITexture>
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

        RefCountPtr<ID3D12Resource> m_resource;

        DescriptorIndex m_rtv = INVALID_DESCRIPTOR_INDEX;

    };
}