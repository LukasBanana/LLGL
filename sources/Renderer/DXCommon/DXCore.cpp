/*
 * DXCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DXCore.h"
#include "ComPtr.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"
#include "../../Core/Vendor.h"
#include <LLGL/Shader.h>
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

static const char* DXErrorToStr(const HRESULT hr)
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

        #ifdef LLGL_DX_ENABLE_D3D12
        LLGL_CASE_TO_STR( D3D12_ERROR_ADAPTER_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D12_ERROR_DRIVER_VERSION_MISMATCH );
        #endif
    }
    return nullptr;
}

[[noreturn]]
static void DXThrowFailure(const HRESULT hr, const char* info)
{
    std::string s;

    if (info)
    {
        s += info;
        s += " (error code = ";
    }
    else
        s += "Direct3D operation failed (error code = ";

    if (auto err = DXErrorToStr(hr))
        s += err;
    else
    {
        s += "0x";
        s += ToHex(hr);
    }

    s += ")";

    throw std::runtime_error(s);
}

void DXThrowIfFailed(const HRESULT hr, const char* info)
{
    if (FAILED(hr))
        DXThrowFailure(hr, info);
}

void DXThrowIfCreateFailed(const HRESULT hr, const char* interfaceName, const char* contextInfo)
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
        DXThrowFailure(hr, s.c_str());
    }
}

void DXThrowIfInvocationFailed(const HRESULT hr, const char* funcName, const char* contextInfo)
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
        DXThrowFailure(hr, s.c_str());
    }
}

BOOL DXBoolean(bool value)
{
    return (value ? TRUE : FALSE);
}

template <typename Cont>
Cont GetBlobDataTmpl(ID3DBlob* blob)
{
    auto data = reinterpret_cast<const char*>(blob->GetBufferPointer());
    auto size = static_cast<std::size_t>(blob->GetBufferSize());

    Cont container;
    {
        container.resize(size);
        std::copy(data, data + size, &container[0]);
    }
    return container;
}

std::string DXGetBlobString(ID3DBlob* blob)
{
    return GetBlobDataTmpl<std::string>(blob);
}

std::vector<char> DXGetBlobData(ID3DBlob* blob)
{
    return GetBlobDataTmpl<std::vector<char>>(blob);
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

static std::uint32_t GetMaxTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 2048;
}

static std::uint32_t GetMaxCubeTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 512;
}

static std::uint32_t GetMaxRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4;
    else                                        return 1;
}

// Returns the HLSL version for the specified Direct3D feature level.
static std::vector<ShadingLanguage> DXGetHLSLVersions(D3D_FEATURE_LEVEL featureLevel)
{
    std::vector<ShadingLanguage> languages;

    languages.push_back(ShadingLanguage::HLSL);
    languages.push_back(ShadingLanguage::HLSL_2_0);

    if (featureLevel >= D3D_FEATURE_LEVEL_9_1 ) { languages.push_back(ShadingLanguage::HLSL_2_0a); }
    if (featureLevel >= D3D_FEATURE_LEVEL_9_2 ) { languages.push_back(ShadingLanguage::HLSL_2_0b); }
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) { languages.push_back(ShadingLanguage::HLSL_3_0); }
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) { languages.push_back(ShadingLanguage::HLSL_4_0); }
    if (featureLevel >= D3D_FEATURE_LEVEL_10_1) { languages.push_back(ShadingLanguage::HLSL_4_1); }
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) { languages.push_back(ShadingLanguage::HLSL_5_0); }
    #ifdef LLGL_DX_ENABLE_D3D12
    if (featureLevel >= D3D_FEATURE_LEVEL_12_0) { languages.push_back(ShadingLanguage::HLSL_5_1); }
    #endif

    return languages;
}

static std::vector<Format> GetDefaultSupportedDXTextureFormats()
{
    return
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
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
void DXGetRenderingCaps(RenderingCapabilities& caps, D3D_FEATURE_LEVEL featureLevel)
{
    const std::uint32_t maxThreadGroups = 65535u;//D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    /* Query common attributes */
    caps.screenOrigin                               = ScreenOrigin::UpperLeft;
    caps.clippingRange                              = ClippingRange::ZeroToOne;
    caps.shadingLanguages                           = DXGetHLSLVersions(featureLevel);
    caps.textureFormats                             = GetDefaultSupportedDXTextureFormats();

    if (featureLevel >= D3D_FEATURE_LEVEL_10_0)
    {
        caps.textureFormats.insert(
            caps.textureFormats.end(),
            { Format::BC4UNorm, Format::BC4SNorm, Format::BC5UNorm, Format::BC5SNorm }
        );
    }

    /* Query features */
    caps.features.hasRenderTargets                  = true;
    caps.features.has3DTextures                     = true;
    caps.features.hasCubeTextures                   = true;
    caps.features.hasArrayTextures                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasCubeArrayTextures              = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    caps.features.hasMultiSampleTextures            = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasTextureViews                   = true;
    caps.features.hasTextureViewSwizzle             = false; // not supported by D3D11
    caps.features.hasBufferViews                    = true;
    caps.features.hasSamplers                       = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.features.hasConstantBuffers                = true;
    caps.features.hasStorageBuffers                 = true;
    caps.features.hasUniforms                       = false; // not supported by D3D11
    caps.features.hasGeometryShaders                = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasTessellationShaders            = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    caps.features.hasTessellatorStage               = caps.features.hasTessellationShaders;
    caps.features.hasComputeShaders                 = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasInstancing                     = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.features.hasOffsetInstancing               = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.features.hasIndirectDrawing                = (featureLevel >= D3D_FEATURE_LEVEL_10_0);//???
    caps.features.hasViewportArrays                 = true;
    caps.features.hasStreamOutputs                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasLogicOp                        = (featureLevel >= D3D_FEATURE_LEVEL_11_1);
    caps.features.hasPipelineStatistics             = true;
    caps.features.hasRenderCondition                = true;

    /* Query limits */
    caps.limits.lineWidthRange[0]                   = 1.0f;
    caps.limits.lineWidthRange[1]                   = 1.0f;
    caps.limits.maxTextureArrayLayers               = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u);
    caps.limits.maxColorAttachments                 = GetMaxRenderTargets(featureLevel);
    caps.limits.maxPatchVertices                    = 32u;
    caps.limits.max1DTextureSize                    = GetMaxTextureDimension(featureLevel);
    caps.limits.max2DTextureSize                    = GetMaxTextureDimension(featureLevel);
    caps.limits.max3DTextureSize                    = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u);
    caps.limits.maxCubeTextureSize                  = GetMaxCubeTextureDimension(featureLevel);
    caps.limits.maxAnisotropy                       = (featureLevel >= D3D_FEATURE_LEVEL_9_2 ? 16u : 2u);
    caps.limits.maxComputeShaderWorkGroups[0]       = maxThreadGroups;
    caps.limits.maxComputeShaderWorkGroups[1]       = maxThreadGroups;
    caps.limits.maxComputeShaderWorkGroups[2]       = (featureLevel >= D3D_FEATURE_LEVEL_11_0 ? maxThreadGroups : 1u);
    caps.limits.maxComputeShaderWorkGroupSize[0]    = 1024u;
    caps.limits.maxComputeShaderWorkGroupSize[1]    = 1024u;
    caps.limits.maxComputeShaderWorkGroupSize[2]    = 1024u;
    caps.limits.maxStreamOutputs                    = 4u;
    caps.limits.maxTessFactor                       = 64u;
    caps.limits.minConstantBufferAlignment          = 256u;
    caps.limits.minSampledBufferAlignment           = 32u;
    caps.limits.minStorageBufferAlignment           = 32u;
}

std::vector<D3D_FEATURE_LEVEL> DXGetFeatureLevels(D3D_FEATURE_LEVEL maxFeatureLevel)
{
    std::vector<D3D_FEATURE_LEVEL> featureLevels =
    {
        #ifdef LLGL_DX_ENABLE_D3D12
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        #endif
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    auto it = std::remove_if(
        featureLevels.begin(), featureLevels.end(),
        [maxFeatureLevel](D3D_FEATURE_LEVEL entry)
        {
            return (entry > maxFeatureLevel);
        }
    );
    featureLevels.erase(it, featureLevels.end());

    return featureLevels;
}

const char* DXFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        #ifdef LLGL_DX_ENABLE_D3D12
        case D3D_FEATURE_LEVEL_12_1:    return "12.1";
        case D3D_FEATURE_LEVEL_12_0:    return "12.0";
        #endif
        case D3D_FEATURE_LEVEL_11_1:    return "11.1";
        case D3D_FEATURE_LEVEL_11_0:    return "11.0";
        case D3D_FEATURE_LEVEL_10_1:    return "10.1";
        case D3D_FEATURE_LEVEL_10_0:    return "10.0";
        case D3D_FEATURE_LEVEL_9_3:     return "9.3";
        case D3D_FEATURE_LEVEL_9_2:     return "9.2";
        case D3D_FEATURE_LEVEL_9_1:     return "9.1";
    }
    return "";
}

const char* DXFeatureLevelToShaderModel(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        #ifdef LLGL_DX_ENABLE_D3D12
        case D3D_FEATURE_LEVEL_12_1:    /*pass*/
        case D3D_FEATURE_LEVEL_12_0:    /*pass*/
        #endif
        case D3D_FEATURE_LEVEL_11_1:    /*pass*/
        case D3D_FEATURE_LEVEL_11_0:    return "5.0";
        case D3D_FEATURE_LEVEL_10_1:    return "4.1";
        case D3D_FEATURE_LEVEL_10_0:    return "4.0";
        case D3D_FEATURE_LEVEL_9_3:     return "3.0";
        case D3D_FEATURE_LEVEL_9_2:     return "2.0b";
        case D3D_FEATURE_LEVEL_9_1:     return "2.0a";
    }
    return "";
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/gg615083(v=vs.85).aspx
UINT DXGetCompilerFlags(int flags)
{
    UINT dxFlags = 0;

    if ((flags & ShaderCompileFlags::Debug) != 0)
        dxFlags |= D3DCOMPILE_DEBUG;

    if ((flags & ShaderCompileFlags::O1) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
    else if ((flags & ShaderCompileFlags::O2) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
    else if ((flags & ShaderCompileFlags::O3) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    else
        dxFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;//D3DCOMPILE_OPTIMIZATION_LEVEL0;

    if ((flags & ShaderCompileFlags::WarnError) != 0)
        dxFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

    return dxFlags;
}

VideoAdapterDescriptor DXGetVideoAdapterDesc(IDXGIAdapter* adapter)
{
    ComPtr<IDXGIOutput> output;

    /* Query adapter description */
    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);

    /* Setup adapter information */
    VideoAdapterDescriptor videoAdapterDesc;

    videoAdapterDesc.name           = std::wstring(desc.Description);
    videoAdapterDesc.vendor         = GetVendorByID(desc.VendorId);
    videoAdapterDesc.videoMemory    = static_cast<uint64_t>(desc.DedicatedVideoMemory);

    /* Enumerate over all adapter outputs */
    for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; ++j)
    {
        /* Get output description */
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        /* Query number of display modes */
        UINT numModes = 0;
        output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);

        /* Query display modes */
        std::vector<DXGI_MODE_DESC> modeDesc(numModes);

        auto hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDesc.data());
        DXThrowIfFailed(hr, "failed to get display mode list with format DXGI_FORMAT_R8G8B8A8_UNORM");

        /* Add output information to the current adapter */
        VideoOutputDescriptor videoOutput;

        for (UINT i = 0; i < numModes; ++i)
        {
            DisplayModeDescriptor displayMode;
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
        videoAdapterDesc.outputs.push_back(videoOutput);

        output.Reset();
    }

    return videoAdapterDesc;
}

Format DXGetSignatureParameterType(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE componentMask)
{
    switch (componentType)
    {
        case D3D_REGISTER_COMPONENT_UINT32:
        {
            switch (componentMask)
            {
                case 0x01: return Format::R32UInt;
                case 0x03: return Format::RG32UInt;
                case 0x07: return Format::RGB32UInt;
                case 0x0F: return Format::RGBA32UInt;
            }
        }
        break;

        case D3D_REGISTER_COMPONENT_SINT32:
        {
            switch (componentMask)
            {
                case 0x01: return Format::R32SInt;
                case 0x03: return Format::RG32SInt;
                case 0x07: return Format::RGB32SInt;
                case 0x0F: return Format::RGBA32SInt;
            }
        }
        break;

        case D3D_REGISTER_COMPONENT_FLOAT32:
        {
            switch (componentMask)
            {
                case 0x01: return Format::R32Float;
                case 0x03: return Format::RG32Float;
                case 0x07: return Format::RGB32Float;
                case 0x0F: return Format::RGBA32Float;
            }
        }
        break;

        default:
        break;
    }
    throw std::runtime_error("failed to map Direct3D signature parameter to LLGL::Format");
}

DXGI_FORMAT DXPickDepthStencilFormat(int depthBits, int stencilBits)
{
    if (depthBits == 32)
    {
        if (stencilBits == 8)
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        else
            return DXGI_FORMAT_D32_FLOAT;
    }
    else if (depthBits == 24 || stencilBits == 8)
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    else
        return DXGI_FORMAT_D16_UNORM;
}


} // /namespace LLGL



// ================================================================================
