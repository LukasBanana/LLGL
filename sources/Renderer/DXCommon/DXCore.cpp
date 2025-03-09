/*
 * DXCore.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <initguid.h> // Comes first to define GUIDs
#include "DXCore.h"
#include "ComPtr.h"
#include "../../Core/Assertion.h"
#include "../../Core/Exception.h"
#include "../../Core/StringUtils.h"
#include "../../Core/MacroUtils.h"
#include "../../Core/Vendor.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/RenderSystemFlags.h>
#include <stdexcept>
#include <algorithm>
#include <d3dcompiler.h>


#ifndef LLGL_BUILD_STATIC_LIB

static HINSTANCE g_moduleHandleDll;

BOOL WINAPI DllMain(
    _In_ HINSTANCE  hInstance,
    _In_ DWORD      reason,
    _In_ LPVOID     /*reserved*/)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            g_moduleHandleDll = hInstance;
            break;
        case DLL_PROCESS_DETACH:
            g_moduleHandleDll = nullptr;
            break;
    }
    return TRUE;
}

#endif // /LLGL_BUILD_STATIC_LIB

namespace LLGL
{


HINSTANCE DXGetDllHandle()
{
    #ifndef LLGL_BUILD_STATIC_LIB
    return g_moduleHandleDll;
    #else
    return GetModuleHandle(nullptr);
    #endif
}

static const char* DXErrorToStr(HRESULT hr)
{
    switch (hr)
    {
        // see https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
        LLGL_CASE_TO_STR( S_OK );
        LLGL_CASE_TO_STR( S_FALSE );
        LLGL_CASE_TO_STR( E_ABORT );
        LLGL_CASE_TO_STR( E_ACCESSDENIED );
        LLGL_CASE_TO_STR( E_FAIL );
        LLGL_CASE_TO_STR( E_HANDLE );
        LLGL_CASE_TO_STR( E_INVALIDARG );
        LLGL_CASE_TO_STR( E_NOINTERFACE );
        LLGL_CASE_TO_STR( E_NOTIMPL );
        LLGL_CASE_TO_STR( E_OUTOFMEMORY );
        LLGL_CASE_TO_STR( E_POINTER );
        LLGL_CASE_TO_STR( E_UNEXPECTED );

        // see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509553(v=vs.85).aspx
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_HUNG );
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_REMOVED );
        LLGL_CASE_TO_STR( DXGI_ERROR_DEVICE_RESET );
        LLGL_CASE_TO_STR( DXGI_ERROR_DRIVER_INTERNAL_ERROR );
        LLGL_CASE_TO_STR( DXGI_ERROR_FRAME_STATISTICS_DISJOINT );
        LLGL_CASE_TO_STR( DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE );
        LLGL_CASE_TO_STR( DXGI_ERROR_INVALID_CALL );
        LLGL_CASE_TO_STR( DXGI_ERROR_MORE_DATA );
        LLGL_CASE_TO_STR( DXGI_ERROR_NONEXCLUSIVE );
        LLGL_CASE_TO_STR( DXGI_ERROR_NOT_CURRENTLY_AVAILABLE );
        LLGL_CASE_TO_STR( DXGI_ERROR_NOT_FOUND );
        LLGL_CASE_TO_STR( DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_REMOTE_OUTOFMEMORY );
        LLGL_CASE_TO_STR( DXGI_ERROR_WAS_STILL_DRAWING );
        LLGL_CASE_TO_STR( DXGI_ERROR_UNSUPPORTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_ACCESS_LOST );
        LLGL_CASE_TO_STR( DXGI_ERROR_WAIT_TIMEOUT );
        LLGL_CASE_TO_STR( DXGI_ERROR_SESSION_DISCONNECTED );
        LLGL_CASE_TO_STR( DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE );
        LLGL_CASE_TO_STR( DXGI_ERROR_CANNOT_PROTECT_CONTENT );
        LLGL_CASE_TO_STR( DXGI_ERROR_ACCESS_DENIED );
        LLGL_CASE_TO_STR( DXGI_ERROR_NAME_ALREADY_EXISTS );
        LLGL_CASE_TO_STR( DXGI_ERROR_SDK_COMPONENT_MISSING );

        // see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476174(v=vs.85).aspx
        LLGL_CASE_TO_STR( D3D10_ERROR_FILE_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS );

        LLGL_CASE_TO_STR( D3D11_ERROR_FILE_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS );
        LLGL_CASE_TO_STR( D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS );
        LLGL_CASE_TO_STR( D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD );

        LLGL_CASE_TO_STR( D3D12_ERROR_ADAPTER_NOT_FOUND );          // WinSDK 10.0.10240.0
        LLGL_CASE_TO_STR( D3D12_ERROR_DRIVER_VERSION_MISMATCH );    // WinSDK 10.0.10240.0
    }
    return nullptr;
}

const char* DXErrorToStrOrHex(HRESULT hr)
{
    if (const char* err = DXErrorToStr(hr))
        return err;
    else
        return IntToHex(hr);
}

[[noreturn]]
static void TrapDXErrorCode(HRESULT hr, const char* details)
{
    const char* errCode = DXErrorToStrOrHex(hr);
    if (details != nullptr && *details != '\0')
        LLGL_TRAP("%s (error code = %s)", details, errCode);
    else
        LLGL_TRAP("Direct3D operation failed (error code = %s)", errCode);
}

void DXThrowIfFailed(HRESULT hr, const char* info)
{
    if (FAILED(hr))
        TrapDXErrorCode(hr, info);
}

void DXThrowIfCastFailed(HRESULT hr, const char* interfaceName, const char* contextInfo)
{
    if (FAILED(hr))
    {
        std::string s;
        {
            s = "failed to interpret object as instance of <";
            s += interfaceName;
            s += '>';
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        TrapDXErrorCode(hr, s.c_str());
    }
}

void DXThrowIfCreateFailed(HRESULT hr, const char* interfaceName, const char* contextInfo)
{
    if (FAILED(hr))
    {
        std::string s;
        {
            s = "failed to create instance of <";
            s += interfaceName;
            s += '>';
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        TrapDXErrorCode(hr, s.c_str());
    }
}

void DXThrowIfInvocationFailed(HRESULT hr, const char* funcName, const char* contextInfo)
{
    if (FAILED(hr))
    {
        std::string s;
        {
            s = "invocation of <";
            s += funcName;
            s += "> failed";
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        TrapDXErrorCode(hr, s.c_str());
    }
}

BOOL DXBoolean(bool value)
{
    return (value ? TRUE : FALSE);
}

template <typename Cont>
Cont GetBlobDataTmpl(ID3DBlob* blob)
{
    auto data = static_cast<const char*>(blob->GetBufferPointer());
    auto size = static_cast<std::size_t>(blob->GetBufferSize());

    Cont container;
    {
        container.resize(size);
        ::memcpy(&container[0], data, size);
    }
    return container;
}

std::string DXGetBlobString(ID3DBlob* blob)
{
    if (blob != nullptr)
        return GetBlobDataTmpl<std::string>(blob);
    else
        return {};
}

std::vector<char> DXGetBlobData(ID3DBlob* blob)
{
    if (blob != nullptr)
        return GetBlobDataTmpl<std::vector<char>>(blob);
    else
        return {};
}

ComPtr<ID3DBlob> DXCreateBlob(const void* data, std::size_t size)
{
    ComPtr<ID3DBlob> blob;
    if (data != nullptr && size > 0)
    {
        D3DCreateBlob(size, &blob);
        ::memcpy(blob->GetBufferPointer(), data, size);
    }
    return blob;
}

ComPtr<ID3DBlob> DXCreateBlob(const std::vector<char>& data)
{
    return DXCreateBlob(data.data(), data.size());
}

ComPtr<ID3DBlob> DXCreateBlobFromResource(int resourceID)
{
    ComPtr<ID3DBlob> blob;

    /* Get module handle */
    HINSTANCE moduleHandle = DXGetDllHandle();

    /* Load resource from binary data (*.rc file) */
    if (HRSRC resource = FindResource(moduleHandle, MAKEINTRESOURCE(resourceID), RT_RCDATA))
    {
        if (HGLOBAL resourceData = LoadResource(moduleHandle, resource))
        {
            /*
            Lock resource data; According to the docs, unlocking is not necessary
            see https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-lockresource
            */
            if (void* data = LockResource(resourceData))
            {
                /* Create blob from locked resource data */
                blob = DXCreateBlob(data, static_cast<std::size_t>(SizeofResource(moduleHandle, resource)));
            }
        }
    }

    return blob;
}

static const Format g_D3DDefaultTextureFormats[] =
{
    Format::A8UNorm,
    Format::R8UNorm,            Format::R8SNorm,            Format::R8UInt,             Format::R8SInt,
    Format::R16UNorm,           Format::R16SNorm,           Format::R16UInt,            Format::R16SInt,            Format::R16Float,
    Format::R32UInt,            Format::R32SInt,            Format::R32Float,
    Format::RG8UNorm,           Format::RG8SNorm,           Format::RG8UInt,            Format::RG8SInt,
    Format::RG16UNorm,          Format::RG16SNorm,          Format::RG16UInt,           Format::RG16SInt,           Format::RG16Float,
    Format::RG32UInt,           Format::RG32SInt,           Format::RG32Float,
    Format::RGB32UInt,          Format::RGB32SInt,          Format::RGB32Float,
    Format::RGBA8UNorm,         Format::RGBA8UNorm_sRGB,    Format::RGBA8SNorm,         Format::RGBA8UInt,          Format::RGBA8SInt,
    Format::RGBA16UNorm,        Format::RGBA16SNorm,        Format::RGBA16UInt,         Format::RGBA16SInt,         Format::RGBA16Float,
    Format::RGBA32UInt,         Format::RGBA32SInt,         Format::RGBA32Float,
    Format::BGRA8UNorm,         Format::BGRA8UNorm_sRGB,
    Format::RGB10A2UNorm,       Format::RGB10A2UInt,        Format::RG11B10Float,       Format::RGB9E5Float,
    Format::D16UNorm,           Format::D32Float,           Format::D24UNormS8UInt,     Format::D32FloatS8X24UInt,
    Format::BC1UNorm,           Format::BC1UNorm_sRGB,
    Format::BC2UNorm,           Format::BC2UNorm_sRGB,
    Format::BC3UNorm,           Format::BC3UNorm_sRGB,
};

void DXGetDefaultSupportedTextureFormats(Format* outFormats, std::size_t* outNumFormats)
{
    if (outFormats != nullptr)
        ::memcpy(outFormats, g_D3DDefaultTextureFormats, sizeof(g_D3DDefaultTextureFormats));
    if (outNumFormats != nullptr)
        *outNumFormats = LLGL_ARRAY_LENGTH(g_D3DDefaultTextureFormats);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
// see https://msdn.microsoft.com/en-us/library/windows/desktop/gg615083(v=vs.85).aspx
UINT DXGetFxcCompilerFlags(int flags)
{
    UINT dxFlags = 0;

    if ((flags & ShaderCompileFlags::Debug) != 0)
        dxFlags |= D3DCOMPILE_DEBUG;

    if ((flags & ShaderCompileFlags::NoOptimization) != 0)
        dxFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    else if ((flags & ShaderCompileFlags::OptimizationLevel1) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
    else if ((flags & ShaderCompileFlags::OptimizationLevel2) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
    else if ((flags & ShaderCompileFlags::OptimizationLevel3) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;

    if ((flags & ShaderCompileFlags::WarningsAreErrors) != 0)
        dxFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

    return dxFlags;
}

static std::vector<VideoAdapterOutputInfo> GetDXGIAdapterOutputInfos(IDXGIAdapter* adapter)
{
    LLGL_ASSERT_PTR(adapter);

    std::vector<VideoAdapterOutputInfo> outputInfos;

    /* Enumerate over all adapter outputs */
    ComPtr<IDXGIOutput> output;
    for (UINT outputIndex = 0; adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++outputIndex)
    {
        /* Get output description */
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        /* Query number of display modes */
        UINT numModes = 0;
        output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);

        /* Query display modes */
        std::vector<DXGI_MODE_DESC> modeDesc(numModes);

        HRESULT hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDesc.data());
        DXThrowIfFailed(hr, "failed to get display mode list with format DXGI_FORMAT_R8G8B8A8_UNORM");

        /* Add output information to the current adapter */
        VideoAdapterOutputInfo videoOutput;

        for_range(i, numModes)
        {
            DisplayMode displayMode;
            {
                displayMode.resolution.width    = modeDesc[i].Width;
                displayMode.resolution.height   = modeDesc[i].Height;
                displayMode.refreshRate         = (modeDesc[i].RefreshRate.Denominator > 0 ? modeDesc[i].RefreshRate.Numerator / modeDesc[i].RefreshRate.Denominator : 0);
            }
            videoOutput.displayModes.push_back(displayMode);
        }

        /* Remove duplicate display modes */
        std::sort(videoOutput.displayModes.begin(), videoOutput.displayModes.end(), CompareSWO);

        videoOutput.displayModes.erase(
            std::unique(videoOutput.displayModes.begin(), videoOutput.displayModes.end()),
            videoOutput.displayModes.end()
        );

        /* Add output to the list and release handle */
        outputInfos.push_back(videoOutput);
    }

    return outputInfos;
}

void DXConvertVideoAdapterInfo(IDXGIAdapter* adapter, const DXGI_ADAPTER_DESC& inDesc, VideoAdapterInfo& outInfo)
{
    outInfo.name        = inDesc.Description;
    outInfo.vendor      = GetVendorByID(inDesc.VendorId);
    outInfo.videoMemory = static_cast<uint64_t>(inDesc.DedicatedVideoMemory);
    outInfo.outputs     = GetDXGIAdapterOutputInfos(adapter);
}

static bool GetDXGIAdapterInfo(IDXGIFactory* factory, long preferredAdapterFlags, VideoAdapterInfo& outInfo, IDXGIAdapter** outPreferredAdatper)
{
    /* Enumerate over all video adapters */
    ComPtr<IDXGIAdapter> adapter;
    for (UINT i = 0; factory->EnumAdapters(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        /* Get adapter descriptor and check if this is either the preferred or the default adapter */
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        const DeviceVendor vendor = GetVendorByID(desc.VendorId);

        const bool isPreferredAdapter = MatchPreferredVendor(vendor, preferredAdapterFlags);
        if (preferredAdapterFlags == 0 || isPreferredAdapter)
        {
            DXConvertVideoAdapterInfo(adapter.Get(), desc, outInfo);
            if (isPreferredAdapter && outPreferredAdatper != nullptr)
                *outPreferredAdatper = adapter.Detach();
            return true;
        }
    }
    return false;
}

VideoAdapterInfo DXGetVideoAdapterInfo(IDXGIFactory* factory, long preferredAdapterFlags, IDXGIAdapter** outPreferredAdatper)
{
    LLGL_ASSERT_PTR(factory);

    constexpr long preferrenceFlags = (RenderSystemFlags::PreferNVIDIA | RenderSystemFlags::PreferAMD | RenderSystemFlags::PreferIntel);

    VideoAdapterInfo info;

    if ((preferredAdapterFlags & preferrenceFlags) != 0)
    {
        if (GetDXGIAdapterInfo(factory, preferredAdapterFlags, info, outPreferredAdatper))
            return info;
    }

    if (GetDXGIAdapterInfo(factory, 0, info, outPreferredAdatper))
        return info;

    return VideoAdapterInfo{};
}

/*
Converts the HLSL component mask to component count.
One and two component shader attributes can be shared with other input/ouput registers as shown in the following example:
  struct VertexIn {
    float2 Pos : POSITION; // Components XY__
    float2 TC  : TEXCOORD; // Components __ZW
  };
This function counts how many bits are in the input value and return it as the component count,
assuming that components are always consequtive, i.e. XY_W for instance is not considered a valid component mask for shader attributes.
*/
static BYTE ComponentMaskToCount(BYTE v)
{
    switch (v)
    {
        case 0x01: // 0001
        case 0x02: // 0010
        case 0x04: // 0100
        case 0x08: // 1000
            return 1;
        case 0x03: // 0011
        case 0x06: // 0110
        case 0x0C: // 1100
            return 2;
        case 0x07: // 0111
        case 0x0E: // 1110
            return 3;
        case 0x0F: // 1111
            return 4;
        default:
            return 0;
    }
}

Format DXGetSignatureParameterType(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE componentMask)
{
    switch (componentType)
    {
        case D3D_REGISTER_COMPONENT_UINT32:
        {
            switch (ComponentMaskToCount(componentMask))
            {
                case 1: return Format::R32UInt;
                case 2: return Format::RG32UInt;
                case 3: return Format::RGB32UInt;
                case 4: return Format::RGBA32UInt;
            }
        }
        break;

        case D3D_REGISTER_COMPONENT_SINT32:
        {
            switch (ComponentMaskToCount(componentMask))
            {
                case 1: return Format::R32SInt;
                case 2: return Format::RG32SInt;
                case 3: return Format::RGB32SInt;
                case 4: return Format::RGBA32SInt;
            }
        }
        break;

        case D3D_REGISTER_COMPONENT_FLOAT32:
        {
            switch (ComponentMaskToCount(componentMask))
            {
                case 1: return Format::R32Float;
                case 2: return Format::RG32Float;
                case 3: return Format::RGB32Float;
                case 4: return Format::RGBA32Float;
            }
        }
        break;

        default:
        break;
    }
    LLGL_TRAP("failed to map Direct3D signature parameter to LLGL::Format");
}

DXGI_FORMAT DXPickDepthStencilFormat(int depthBits, int stencilBits)
{
    /* Only return unknown format if depth-stencil is explicitly disabled */
    if (depthBits == 0 && stencilBits == 0)
        return DXGI_FORMAT_UNKNOWN;

    if (depthBits == 32)
    {
        if (stencilBits == 8)
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        else
            return DXGI_FORMAT_D32_FLOAT;
    }
    else if (depthBits == 16)
        return DXGI_FORMAT_D16_UNORM;

    /* Return standard D24S8 depth buffer format by default */
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

bool DXGetFullscreenState(IDXGISwapChain* swapChain)
{
    BOOL fullscreenState = FALSE;
    HRESULT hr = swapChain->GetFullscreenState(&fullscreenState, nullptr);
    DXThrowIfFailed(hr, "failed to get fullscreen state");
    return (fullscreenState != FALSE);
}

GUID DXGetD3DDebugObjectNameGUID()
{
    return WKPDID_D3DDebugObjectName;
}


} // /namespace LLGL



// ================================================================================
