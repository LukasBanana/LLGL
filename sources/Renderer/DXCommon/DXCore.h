/*
 * DXCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DX_CORE_H
#define LLGL_DX_CORE_H


#include <LLGL/ColorRGBA.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/VideoAdapter.h>
#include <LLGL/Image.h>
#include <dxgi.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <d3dcommon.h>


namespace LLGL
{


/* ----- Structure ----- */

//TODO: replace by global struct "ClearValue"
// D3D clear view state structure (with color and depth-stencil clear values).
struct D3DClearState
{
    ColorRGBAf  color   = { 0.0f, 0.0f, 0.0f, 0.0f };
    FLOAT       depth   = 1.0f;
    UINT8       stencil = 0;
};

// Small descriptor structure for internal D3D texture format.
struct D3DTextureFormatDescriptor
{
    ImageFormat format;
    DataType    dataType;
};


/* ----- Functions ----- */

// Throws an std::runtime_error exception if 'hr' is not S_OK.
void DXThrowIfFailed(const HRESULT hr, const char* info);

// Returns the blob data as string.
std::string DXGetBlobString(ID3DBlob* blob);

// Returns the blob data as char vector.
std::vector<char> DXGetBlobData(ID3DBlob* blob);

// Returns the rendering capabilites of the specified Direct3D feature level.
void DXGetRenderingCaps(RenderingCaps& caps, D3D_FEATURE_LEVEL featureLevel);

// Returns the list of all supported Direct3D feature levels.
std::vector<D3D_FEATURE_LEVEL> DXGetFeatureLevels(D3D_FEATURE_LEVEL maxFeatureLevel);

// Returns the specified feature level as version string.
std::string DXFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel);

// Returns the specified feature level as HLSL shader model version string.
std::string DXFeatureLevelToShaderModel(D3D_FEATURE_LEVEL featureLevel);

// Returns the compiler flags for the 'ShaderCompileFlags' enumeration values.
UINT DXGetCompilerFlags(int flags);

// Returns the disassembler flags for the 'ShaderDisassembleFlags' enumeration values.
UINT DXGetDisassemblerFlags(int flags);

// Returns the video adapter descriptor from the specified DXGI adapter.
VideoAdapterDescriptor DXGetVideoAdapterDesc(IDXGIAdapter* adapter);

// Returns the LLGL format and data type for the specified DXGI format.
D3DTextureFormatDescriptor DXGetTextureFormatDesc(DXGI_FORMAT format);


} // /namespace LLGL


#endif



// ================================================================================
