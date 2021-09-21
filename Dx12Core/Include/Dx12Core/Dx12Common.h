#pragma once

#include "d3dx12.h"

#include <dxgi1_6.h>
#include <d3d12.h>

#ifdef _DEBUG
	#include <dxgidebug.h>
#endif

#include <locale>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>


#include "RefCountPtr.h"
#include "Log.h"

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#define INVALID_DESCRIPTOR_INDEX ~0u

namespace Dx12Core
{
    typedef uint32_t DescriptorIndex;

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }

    inline std::string NarrowString(const wchar_t* WideStr)
    {
        std::string NarrowStr;

        const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(std::wstringstream().getloc());
        for (auto CurrWChar = WideStr; *CurrWChar != 0; ++CurrWChar)
            NarrowStr.push_back(ctfacet.narrow(*CurrWChar, 0));

        return NarrowStr;
    }

    // Clamp a value between a min and max range.
    template<typename T>
    constexpr const T& clamp(const T& val, const T& min, const T& max)
    {
        return val < min ? min : val > max ? max : val;
    }

    // TODO Make immutable
    struct Dx12Context
    {
        RefCountPtr<IDXGIFactory6> Factory;
        RefCountPtr<ID3D12Device> Device;
        RefCountPtr<ID3D12Device2> Device2;
        RefCountPtr<ID3D12Device5> Device5;

        RefCountPtr<IDXGIAdapter> GpuAdapter;

        bool IsDxrSupported = false;
        bool IsRayQuerySupported = false;
        bool IsRenderPassSupported = false;
        bool IsVariableRateShadingSupported = false;
        bool IsMeshShadingSupported = false;
        bool IsUnderGraphicsDebugger = false;
    };

    // A type cast that is safer than static_cast in debug builds, and is a simple static_cast in release builds.
// Used for downcasting various ISomething* pointers to their implementation classes in the backends.
    template <typename T, typename U>
    T SafeCast(U u)
    {
        static_assert(!std::is_same<T, U>::value, "Redundant checked_cast");
#ifdef _DEBUG
        if (!u) return nullptr;
        T t = dynamic_cast<T>(u);
        if (!t) assert(!"Invalid type cast");  // NOLINT(clang-diagnostic-string-conversion)
        return t;
#else
        return static_cast<T>(u);
#endif
    }
}