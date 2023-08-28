/*
 * DXCInstance.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DXC_INSTANCE_H
#define LLGL_DXC_INSTANCE_H


#include "../ComPtr.h"
#include <d3d12shader.h>
#include <vector>


namespace LLGL
{


// Loads the DirectXShaderCompiler (DXC).
HRESULT DXLoadDxcompilerInterface();

// Returns the compiler arguments for the 'ShaderCompileFlags' enumeration values for the DirectXShaderCompiler (DXC).
std::vector<LPCWSTR> DXGetDxcCompilerArgs(int flags);

// Compiles the specified shader source to DXIL byte code with the DirectXShaderCompiler (DXC).
HRESULT DXCompileShaderToDxil(
    const char* source,
    std::size_t sourceLength,
    LPCWSTR*    args,
    std::size_t numArgs,
    ID3DBlob**  outByteCode,
    ID3DBlob**  outErrors
);

// Reflects the specified DXIL shader byte code.
HRESULT DXReflectDxilShader(
    ID3DBlob*                   byteCode,
    ID3D12ShaderReflection**    outReflection
);


} // /namespace LLGL


#endif



// ================================================================================
