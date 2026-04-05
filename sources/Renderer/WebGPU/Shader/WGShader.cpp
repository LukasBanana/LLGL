/*
 * WGShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGShader.h"
#include "../WGCore.h"
#include "../../ShaderUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/StringUtils.h"


namespace LLGL
{


WGShader::WGShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    BuildShader(instance, device, desc);
}

const Report* WGShader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

bool WGShader::Reflect(ShaderReflection& reflection) const
{
    return false; //todo
}


/*
 * ======= Private: =======
 */

static void OnShaderCompilationInfo(WGPUCompilationInfoRequestStatus status, const WGPUCompilationInfo* compilationInfo, void* userdata1, void* userdata2)
{
    Report* report = reinterpret_cast<Report*>(userdata1);
    ShaderSourceContext* sourceContext = static_cast<ShaderSourceContext*>(userdata2);
    if (status == WGPUCompilationInfoRequestStatus_Success)
    {
        /* Append all messages to the report */
        for (std::uint32_t i = 0; i < compilationInfo->messageCount; ++i)
        {
            const WGPUCompilationMessage& msg = compilationInfo->messages[i];

            const ShaderSourceLine sourceLine = sourceContext->GetSourceLine(
                static_cast<unsigned>(msg.lineNum - 1),
                static_cast<unsigned>(msg.linePos - 1),
                static_cast<unsigned>(msg.length)
            );

            if (msg.type == WGPUCompilationMessageType_Info)
            {
                report->Printf(
                    "%u:%u info: %.*s\n%.*s\n%s\n",
                    static_cast<unsigned>(msg.lineNum),
                    static_cast<unsigned>(msg.linePos),
                    static_cast<int>(msg.message.length),
                    msg.message.data,
                    static_cast<int>(sourceLine.lineText.size()),
                    sourceLine.lineText.data(),
                    sourceLine.lineMarker.c_str()
                );
            }
            else
            {
                report->Errorf(
                    "%u:%u %s: %.*s\n%.*s\n%s\n",
                    static_cast<unsigned>(msg.lineNum),
                    static_cast<unsigned>(msg.linePos),
                    (msg.type == WGPUCompilationMessageType_Warning ? "warning" : "error"),
                    static_cast<int>(msg.message.length),
                    msg.message.data,
                    static_cast<int>(sourceLine.lineText.size()),
                    sourceLine.lineText.data(),
                    sourceLine.lineMarker.c_str()
                );
            }
        }
    }
    else
    {
        /* Compilation info request failed */
        report->Errorf("failed to retrieve shader compilation information\n");
    }
}

bool WGShader::BuildShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& shaderDesc)
{
    /* Get shader content */
    std::vector<char>   binaryContent;
    ArrayView<char>     binaryContentView;

    std::string         textContent;
    ShaderSourceContext sourceContext;

    switch (shaderDesc.sourceType)
    {
        case ShaderSourceType::CodeString:
            /* Load binary from buffer */
            sourceContext.sourceText = StringView{ shaderDesc.source, shaderDesc.sourceSize };
            break;

        case ShaderSourceType::CodeFile:
            /* Load text from file */
            textContent = ReadFileString(shaderDesc.source);
            sourceContext.sourceText = textContent;
            break;

        case ShaderSourceType::BinaryBuffer:
            /* Load binary from buffer */
            binaryContentView = ArrayView<char>{ shaderDesc.source, shaderDesc.sourceSize };
            break;

        case ShaderSourceType::BinaryFile:
            /* Load binary from file */
            binaryContent = ReadFileBuffer(shaderDesc.source);
            binaryContentView = binaryContent;
            break;
    }

    /* Setup WebGPU shader source descriptor */
    WGPUShaderSourceWGSL sourceWGSL;
    WGPUShaderSourceSPIRV sourceSPIRV;

    if (IsShaderSourceCode(shaderDesc.sourceType))
    {
        sourceWGSL.chain        = { nullptr, WGPUSType_ShaderSourceWGSL };
        sourceWGSL.code.data    = sourceContext.sourceText.data();
        sourceWGSL.code.length  = sourceContext.sourceText.size();
    }
    else
    {
        sourceSPIRV.chain       = { nullptr, WGPUSType_ShaderSourceSPIRV };
        sourceSPIRV.codeSize    = static_cast<std::uint32_t>(binaryContentView.size());
        sourceSPIRV.code        = reinterpret_cast<const std::uint32_t*>(binaryContentView.data());
    }

    /* Create WebGPU shader module */
    WGPUShaderModuleDescriptor moduleDesc;
    {
        if (IsShaderSourceCode(shaderDesc.sourceType))
            moduleDesc.nextInChain = &(sourceWGSL.chain);
        else
            moduleDesc.nextInChain = &(sourceSPIRV.chain);
        moduleDesc.label = WGPU_STRING_VIEW_INIT;
    }
    shaderModule_ = wgpuDeviceCreateShaderModule(device, &moduleDesc);
    LLGL_ASSERT_PTR(shaderModule_);

    /* Get async compilation information immediately */
    WGPUCompilationInfoCallbackInfo compilationInfoCallback;
    {
        compilationInfoCallback.nextInChain = nullptr;
        compilationInfoCallback.mode        = WGPUCallbackMode_WaitAnyOnly;
        compilationInfoCallback.callback    = OnShaderCompilationInfo;
        compilationInfoCallback.userdata1   = &report_;
        compilationInfoCallback.userdata2   = &sourceContext;
    }
    WGPUFutureWaitInfo waitInfo;
    {
        waitInfo.future     = wgpuShaderModuleGetCompilationInfo(shaderModule_, compilationInfoCallback);
        waitInfo.completed  = WGPU_FALSE;
    }
    const WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT64_MAX);

    if (waitStatus != WGPUWaitStatus_Success)
    {
        report_.Errorf("failed to request WebGPU shader compilation result (%s)", ToString(waitStatus));
        return false;
    }

    return true;
}


} // /namespace LLGL



// ================================================================================
