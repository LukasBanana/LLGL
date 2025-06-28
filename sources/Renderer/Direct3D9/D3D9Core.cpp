/*
 * D3D9Core.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Core.h"
#include "../../Core/Assertion.h"
#include "../../Core/Exception.h"
#include "../../Core/StringUtils.h"
#include <LLGL/ShaderFlags.h>
#include <d3dcompiler.h>


namespace LLGL
{


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
    }
    return nullptr;
}

const char* D3DErrorToStrOrHex(HRESULT hr)
{
    if (const char* err = DXErrorToStr(hr))
        return err;
    else
        return IntToHex(static_cast<UINT>(hr));
}

[[noreturn]]
static void TrapD3DErrorCode(HRESULT hr, const char* details)
{
    const char* errCode = D3DErrorToStrOrHex(hr);
    if (details != nullptr && *details != '\0')
        LLGL_TRAP("%s (error code = %s)", details, errCode);
    else
        LLGL_TRAP("Direct3D operation failed (error code = %s)", errCode);
}

void D3DThrowIfFailed(HRESULT hr, const char* info)
{
    if (FAILED(hr))
        TrapD3DErrorCode(hr, info);
}

void D3DThrowIfCreateFailed(HRESULT hr, const char* interfaceName, const char* contextInfo)
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
        TrapD3DErrorCode(hr, s.c_str());
    }
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
// see https://msdn.microsoft.com/en-us/library/windows/desktop/gg615083(v=vs.85).aspx
UINT D3DGetFxcCompilerFlags(int flags)
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

std::string D3DGetBlobString(ID3DBlob* blob)
{
    if (blob != nullptr)
        return GetBlobDataTmpl<std::string>(blob);
    else
        return {};
}

std::vector<char> D3DGetBlobData(ID3DBlob* blob)
{
    if (blob != nullptr)
        return GetBlobDataTmpl<std::vector<char>>(blob);
    else
        return {};
}

ComPtr<ID3DBlob> D3DCreateBlob(const void* data, std::size_t size)
{
    ComPtr<ID3DBlob> blob;
    if (data != nullptr && size > 0)
    {
        D3DCreateBlob(size, &blob);
        ::memcpy(blob->GetBufferPointer(), data, size);
    }
    return blob;
}

ComPtr<ID3DBlob> D3DCreateBlob(const std::vector<char>& data)
{
    return D3DCreateBlob(data.data(), data.size());
}


} // /namespace LLGL



// ================================================================================
