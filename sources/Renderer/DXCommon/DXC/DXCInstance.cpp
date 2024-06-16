/*
 * DXCInstance.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DXCInstance.h"
#include <LLGL/ShaderFlags.h>
#include "../../../Platform/Module.h"
#include <dxcapi.h>


namespace LLGL
{


struct DXCInstance
{
    std::unique_ptr<Module> module;
    DxcCreateInstanceProc   dxcCreateInstance   = nullptr;
};

static DXCInstance g_DXCInstance;

HRESULT DXLoadDxcompilerInterface()
{
    /* Early exit if we already loaded the interface */
    if (g_DXCInstance.module)
        return S_OK;

    /* Try to load dxcompiler.dll */
    g_DXCInstance.module = Module::Load("dxcompiler.dll");
    if (g_DXCInstance.module == nullptr)
        return E_FAIL;

    /* Load "DxcCreateInstance" procedure */
    g_DXCInstance.dxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(g_DXCInstance.module->LoadProcedure("DxcCreateInstance"));
    if (g_DXCInstance.dxcCreateInstance == nullptr)
    {
        g_DXCInstance.module.reset();
        return E_FAIL;
    }

    return S_OK;
}

std::vector<LPCWSTR> DXGetDxcCompilerArgs(int flags)
{
    std::vector<LPCWSTR> dxArgs;

    if ((flags & ShaderCompileFlags::Debug) != 0)
        dxArgs.push_back(DXC_ARG_DEBUG);

    if ((flags & ShaderCompileFlags::NoOptimization) != 0)
        dxArgs.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
    else if ((flags & ShaderCompileFlags::OptimizationLevel1) != 0)
        dxArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL1);
    else if ((flags & ShaderCompileFlags::OptimizationLevel2) != 0)
        dxArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL2);
    else if ((flags & ShaderCompileFlags::OptimizationLevel3) != 0)
        dxArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);

    if ((flags & ShaderCompileFlags::WarningsAreErrors) != 0)
        dxArgs.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

    return dxArgs;
}

HRESULT DXCompileShaderToDxil(
    const char* source,
    std::size_t sourceLength,
    LPCWSTR*    args,
    std::size_t numArgs,
    ID3DBlob**  outByteCode,
    ID3DBlob**  outErrors)
{
    if (g_DXCInstance.dxcCreateInstance == nullptr)
        return E_FAIL;

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr        = source;
    sourceBuffer.Size       = sourceLength;
    sourceBuffer.Encoding   = DXC_CP_ACP;

    ComPtr<IDxcCompiler3> compiler;
    HRESULT hr = g_DXCInstance.dxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hr))
        return hr;

    ComPtr<IDxcUtils> utils;
    hr = g_DXCInstance.dxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    if (FAILED(hr))
        return hr;

    ComPtr<IDxcIncludeHandler> includeHandler;
    hr = utils->CreateDefaultIncludeHandler(&includeHandler);
    if (FAILED(hr))
        return hr;

    ComPtr<IDxcResult> result;
    hr = compiler->Compile(
        &sourceBuffer,
        args,
        static_cast<UINT32>(numArgs),
        includeHandler.Get(),
        IID_PPV_ARGS(&result)
    );
    if (FAILED(hr))
        return hr;

    HRESULT compileResult = S_OK;
    hr = result->GetStatus(&compileResult);
    if (FAILED(hr))
        return hr;

    hr = compileResult;

    if (SUCCEEDED(compileResult))
    {
        hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(outByteCode), nullptr);
        if (FAILED(hr))
            return hr;
    }

    hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(outErrors), nullptr);
    if (FAILED(hr))
        return hr;

    return compileResult;
}

HRESULT DXReflectDxilShader(
    ID3DBlob*                   byteCode,
    ID3D12ShaderReflection**    outReflection)
{
    if (g_DXCInstance.dxcCreateInstance == nullptr)
        return E_FAIL;
    if (byteCode == nullptr)
        return E_INVALIDARG;

    ComPtr<IDxcUtils> dxcUtils;
    HRESULT hr = g_DXCInstance.dxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    if (FAILED(hr))
        return hr;

    DxcBuffer reflectionBuffer;
    reflectionBuffer.Ptr        = byteCode->GetBufferPointer();
    reflectionBuffer.Size       = byteCode->GetBufferSize();
    reflectionBuffer.Encoding   = DXC_CP_ACP;

    hr = dxcUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(outReflection));
    if (FAILED(hr))
        return hr;

    return S_OK;
}


} // /namespace LLGL



// ================================================================================
