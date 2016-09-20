/*
 * DXCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../DXCommon/DXCore.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"
#include <LLGL/Shader.h>
#include <stdexcept>
#include <algorithm>
#include <d3dcompiler.h>


namespace LLGL
{


std::string DXErrorToStr(const HRESULT errorCode)
{
    switch (errorCode)
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
        LLGL_CASE_TO_STR( D3D12_ERROR_ADAPTER_NOT_FOUND );
        LLGL_CASE_TO_STR( D3D12_ERROR_DRIVER_VERSION_MISMATCH );
    }
    return ToHex(errorCode);
}

void DXThrowIfFailed(const HRESULT errorCode, const std::string& info)
{
    if (FAILED(errorCode))
        throw std::runtime_error(info + " (error code = " + DXErrorToStr(errorCode) + ")");
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

static int GetMaxTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 2048;
}

static int GetMaxCubeTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 512;
}

static unsigned int GetMaxRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4;
    else                                        return 1;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
void DXGetRenderingCaps(RenderingCaps& caps, D3D_FEATURE_LEVEL featureLevel)
{
    unsigned int maxThreadGroups = 65535;//D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    caps.screenOrigin                   = ScreenOrigin::UpperLeft;
    caps.clippingRange                  = ClippingRange::ZeroToOne;
    caps.hasRenderTargets               = true;
    caps.has3DTextures                  = true;
    caps.hasCubeTextures                = true;
    caps.hasTextureArrays               = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasCubeTextureArrays           = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    caps.hasSamplers                    = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasConstantBuffers             = true;
    caps.hasStorageBuffers              = true;
    caps.hasUniforms                    = false;
    caps.hasGeometryShaders             = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasTessellationShaders         = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    caps.hasComputeShaders              = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasInstancing                  = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasOffsetInstancing            = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasViewportArrays              = true;
    caps.hasConservativeRasterization   = (featureLevel >= D3D_FEATURE_LEVEL_11_1);
    caps.maxNumTextureArrayLayers       = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048 : 256);
    caps.maxNumRenderTargetAttachments  = GetMaxRenderTargets(featureLevel);
    caps.maxConstantBufferSize          = 16384;
    caps.maxPatchVertices               = 32;
    caps.max1DTextureSize               = GetMaxTextureDimension(featureLevel);
    caps.max2DTextureSize               = GetMaxTextureDimension(featureLevel);
    caps.max3DTextureSize               = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048 : 256);
    caps.maxCubeTextureSize             = GetMaxCubeTextureDimension(featureLevel);
    caps.maxAnisotropy                  = (featureLevel >= D3D_FEATURE_LEVEL_9_2 ? 16 : 2);
    caps.maxNumComputeShaderWorkGroups  = { maxThreadGroups, maxThreadGroups, (featureLevel >= D3D_FEATURE_LEVEL_11_0 ? maxThreadGroups : 1u) };
    caps.maxComputeShaderWorkGroupSize  = { 1024, 1024, 1024 };
    caps.hasHLSL                        = true;
}

ShadingLanguage DXGetHLSLVersion(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return ShadingLanguage::HLSL_5_0;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_1) return ShadingLanguage::HLSL_4_1;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return ShadingLanguage::HLSL_4_0;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return ShadingLanguage::HLSL_3_0;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_2 ) return ShadingLanguage::HLSL_2_0b;
    else                                        return ShadingLanguage::HLSL_2_0a;
}

std::vector<D3D_FEATURE_LEVEL> DXGetFeatureLevels(D3D_FEATURE_LEVEL maxFeatureLevel)
{
    std::vector<D3D_FEATURE_LEVEL> featureLeves =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    auto it = std::remove_if(
        featureLeves.begin(), featureLeves.end(),
        [maxFeatureLevel](D3D_FEATURE_LEVEL entry)
        {
            return (entry > maxFeatureLevel);
        }
    );
    featureLeves.erase(it, featureLeves.end());

    return featureLeves;
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

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607326(v=vs.85).aspx
UINT DXGetDisassemblerFlags(int flags)
{
    UINT dxFlags = 0;

    if ((flags & ShaderDisassembleFlags::InstructionOnly) != 0)
        dxFlags |= D3D_DISASM_INSTRUCTION_ONLY;

    return dxFlags;
}


} // /namespace LLGL



// ================================================================================
