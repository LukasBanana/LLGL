/*
 * DXCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DX_CORE_H
#define LLGL_DX_CORE_H


#include <LLGL/ColorRGBA.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/VideoAdapter.h>
#include <LLGL/ImageFlags.h>
#include "ComPtr.h"
#include <dxgi.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <d3dcommon.h>


namespace LLGL
{


/* ----- Structure ----- */

// Small descriptor structure for internal D3D texture format.
struct D3DTextureFormatDescriptor
{
    ImageFormat format;
    DataType    dataType;
};


/* ----- Functions ----- */

// Returns the DLL instance handle of this module.
HINSTANCE DXGetDllHandle();

// Throws an std::runtime_error exception if 'hr' is not S_OK.
void DXThrowIfFailed(const HRESULT hr, const char* info);

// Throws an std::runtime_error exception if 'hr' is not S_OK, with an info about the failed interface creation.
void DXThrowIfCreateFailed(const HRESULT hr, const char* interfaceName, const char* contextInfo = nullptr);

// Throws an std::runtime_error exception if 'hr' is not S_OK, with an info about the failed function call.
void DXThrowIfInvocationFailed(const HRESULT hr, const char* funcName, const char* contextInfo = nullptr);

// Returns the specified value as a DirectX BOOL type.
BOOL DXBoolean(bool value);

// Returns the blob data as string.
std::string DXGetBlobString(ID3DBlob* blob);

// Returns the blob data as char vector.
std::vector<char> DXGetBlobData(ID3DBlob* blob);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> DXCreateBlob(const void* data, std::size_t size);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> DXCreateBlob(const std::vector<char>& data);

// Returns a blob that was created from a resource (*.rc files).
ComPtr<ID3DBlob> DXCreateBlobFromResource(int resourceID);

// Returns the rendering capabilites of the specified Direct3D feature level.
void DXGetRenderingCaps(RenderingCapabilities& caps, D3D_FEATURE_LEVEL featureLevel);

// Returns the list of all supported Direct3D feature levels.
std::vector<D3D_FEATURE_LEVEL> DXGetFeatureLevels(D3D_FEATURE_LEVEL maxFeatureLevel);

// Returns the specified feature level as version string.
const char* DXFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel);

// Returns the specified feature level as HLSL shader model version string.
const char* DXFeatureLevelToShaderModel(D3D_FEATURE_LEVEL featureLevel);

// Returns the compiler flags for the 'ShaderCompileFlags' enumeration values.
UINT DXGetCompilerFlags(int flags);

// Returns the video adapter descriptor from the specified DXGI adapter.
VideoAdapterDescriptor DXGetVideoAdapterDesc(IDXGIAdapter* adapter);

// Returns the format for the specified signature parameter type (by its component type and mask).
Format DXGetSignatureParameterType(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE componentMask);

// Returns a suitable DXGI format for the specified depth-stencil mode.
DXGI_FORMAT DXPickDepthStencilFormat(int depthBits, int stencilBits);


} // /namespace LLGL


#endif



// ================================================================================
