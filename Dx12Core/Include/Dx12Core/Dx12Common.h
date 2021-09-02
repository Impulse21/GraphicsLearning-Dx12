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

#include "RefCountPtr.h"

#include "Log.h"

namespace Dx12Core
{
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
}