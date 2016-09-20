/*
 * DXCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DX_CORE_H__
#define __LLGL_DX_CORE_H__


#include <LLGL/ColorRGBA.h>
#include <LLGL/RenderSystemFlags.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <d3dcommon.h>


namespace LLGL
{


/* ----- Structure ----- */

struct D3DClearState
{
    ColorRGBAf  color   = { 0.0f, 0.0f, 0.0f, 0.0f };
    FLOAT       depth   = 0.0f;
    UINT8       stencil = 0;
};


/* ----- Functions ----- */

// Converts the DX error code into a string.
std::string DXErrorToStr(const HRESULT errorCode);

// Throws an std::runtime_error exception of 'errorCode' is not S_OK.
void DXThrowIfFailed(const HRESULT errorCode, const std::string& info);

// Returns the blob data as string.
std::string DXGetBlobString(ID3DBlob* blob);

// Returns the blob data as char vector.
std::vector<char> DXGetBlobData(ID3DBlob* blob);

// Returns the rendering capabilites of the specified Direct3D feature level.
void DXGetRenderingCaps(RenderingCaps& caps, D3D_FEATURE_LEVEL featureLevel);

// Returns the HLSL version for the specified Direct3D feature level.
ShadingLanguage DXGetHLSLVersion(D3D_FEATURE_LEVEL featureLevel);

// Returns the list of all supported Direct3D feature levels.
std::vector<D3D_FEATURE_LEVEL> DXGetFeatureLevels(D3D_FEATURE_LEVEL maxFeatureLevel);

// Returns the compiler flags for the 'ShaderCompileFlags' enumeration values.
UINT DXGetCompilerFlags(int flags);

// Returns the disassembler flags for the 'ShaderDisassembleFlags' enumeration values.
UINT DXGetDisassemblerFlags(int flags);


} // /namespace LLGL


#endif



// ================================================================================
