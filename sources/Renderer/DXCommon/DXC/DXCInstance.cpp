/*
 * DXCInstance.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DXCInstance.h"
#include "../DXCore.h"
#include "../ComPtr.h"
#include "../../../Platform/Module.h"
#include "../../../Core/Assertion.h"
#include <LLGL/ShaderFlags.h>
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

// DXC include handler wrapper to forward `#include`-directives from D3D's COM interface to LLGL's IncludeHandler interface.
class DXCIncludeHandler final : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IDxcIncludeHandler>
{

    public:

        DXCIncludeHandler(IDxcUtils* utils, IncludeHandler* forwardIncludeHandler, Report& outReport) :
            utils_                 { utils                 },
            outReport_             { outReport             },
            forwardIncludeHandler_ { forwardIncludeHandler }
        {
        }

        STDMETHOD(LoadSource)(LPCWSTR filename, IDxcBlob** outIncludeSource) override
        {
            LLGL_ASSERT_PTR(forwardIncludeHandler_);

            Blob fileContent;
            if (forwardIncludeHandler_->Include(filename, fileContent, outReport_))
            {
                if (outIncludeSource != nullptr)
                {
                    /* Create blob from file content */
                    ComPtr<IDxcBlobEncoding> blobEncoding;
                    HRESULT hr = utils_->CreateBlob(fileContent.GetData(), static_cast<UINT32>(fileContent.GetSize()), DXC_CP_ACP, &blobEncoding);
                    if (FAILED(hr))
                        return hr;

                    /* Pass on to output blob */
                    hr = utils_->CreateBlobFromBlob(blobEncoding.Get(), 0, static_cast<UINT32>(blobEncoding->GetBufferSize()), outIncludeSource);
                    if (FAILED(hr))
                        return hr;
                }
                return S_OK;
            }

            return E_FAIL;
        }

    private:

        IDxcUtils*      utils_                  = nullptr;
        Report&         outReport_;
        IncludeHandler* forwardIncludeHandler_  = nullptr;

};

static HRESULT MakeDXCIncludeHandler(
    IDxcUtils*                  utils,
    IncludeHandler*             inIncludeHandler,
    Report&                     inIncludeHandlerReport,
    ComPtr<IDxcIncludeHandler>& outIncludeHandler)
{
    if (inIncludeHandler != nullptr)
    {
        outIncludeHandler = Microsoft::WRL::Make<DXCIncludeHandler>(utils, inIncludeHandler, inIncludeHandlerReport);
        return (outIncludeHandler ? S_OK : E_FAIL);
    }
    else
        return utils->CreateDefaultIncludeHandler(&outIncludeHandler);
}

HRESULT DXCompileShaderToDxil(
    const char*     source,
    std::size_t     sourceLength,
    LPCWSTR*        args,
    std::size_t     numArgs,
    ID3DBlob**      outByteCode,
    ID3DBlob**      outErrors,
    IncludeHandler* includeHandler,
    Report*         includeHandlerReport)
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

    Report includeHandlerNullReport;
    ComPtr<IDxcIncludeHandler> includeHandlerWrapper;
    hr = MakeDXCIncludeHandler(
        utils.Get(),
        includeHandler,
        (includeHandlerReport != nullptr ? *includeHandlerReport : includeHandlerNullReport),
        includeHandlerWrapper
    );
    if (FAILED(hr))
        return hr;

    ComPtr<IDxcResult> result;
    hr = compiler->Compile(
        &sourceBuffer,
        args,
        static_cast<UINT32>(numArgs),
        includeHandlerWrapper.Get(),
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
