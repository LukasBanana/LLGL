/*
 * DXCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DX_CORE_H__
#define __LLGL_DX_CORE_H__


#include <LLGL/ColorRGBA.h>
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

//! Converts the DX error code into a string.
std::string DXErrorToStr(const HRESULT errorCode);

//! Throws an std::runtime_error exception of 'errorCode' is not S_OK.
void DXThrowIfFailed(const HRESULT errorCode, const std::string& info);

//! Returns the blob data as string.
std::string DXGetBlobString(ID3DBlob* blob);

//! Returns the blob data as char vector.
std::vector<char> DXGetBlobData(ID3DBlob* blob);


} // /namespace LLGL


#endif



// ================================================================================
