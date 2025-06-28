/*
 * D3D9Core.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_CORE_H
#define LLGL_D3D9_CORE_H


#include "../DXCommon/ComPtr.h"
#include "Direct3D9.h"
#include <d3dcommon.h>
#include <string>
#include <vector>


namespace LLGL
{


/* ----- Functions ----- */

// Returns a string representation for the specified HRESULT error code.
const char* D3DErrorToStrOrHex(HRESULT hr);

// Traps the runtime if 'hr' is not S_OK.
void D3DThrowIfFailed(HRESULT hr, const char* info);

// Traps the runtime if 'hr' is not S_OK, with an info about the failed interface creation.
void D3DThrowIfCreateFailed(HRESULT hr, const char* interfaceName, const char* contextInfo = nullptr);

// Returns the compiler flags for the 'ShaderCompileFlags' enumeration values for the DirectX Effects Compiler (FXC).
UINT D3DGetFxcCompilerFlags(int flags);

// Returns the blob data as string.
std::string D3DGetBlobString(ID3DBlob* blob);

// Returns the blob data as char vector.
std::vector<char> D3DGetBlobData(ID3DBlob* blob);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> D3DCreateBlob(const void* data, std::size_t size);

// Returns a blob and copies the specified data into the blob.
ComPtr<ID3DBlob> D3DCreateBlob(const std::vector<char>& data);


} // /namespace LLGL


#endif



// ================================================================================
