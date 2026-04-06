/*
 * WGShaderModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGShaderModule.h"
#include "../WGCore.h"
#include "../../ShaderUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/StringUtils.h"


namespace LLGL
{


WGShaderModule::WGShaderModule(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext) :
    shaderModule_ { wgpuShaderModuleRelease }
{
    BuildShader(instance, device, sourceContext);
}

static void OnShaderCompilationInfo(WGPUCompilationInfoRequestStatus status, const WGPUCompilationInfo* compilationInfo, void* userdata1, void* userdata2)
{
    Report* report = reinterpret_cast<Report*>(userdata1);
    const ShaderSourceContext* sourceContext = static_cast<const ShaderSourceContext*>(userdata2);

    if (status != WGPUCompilationInfoRequestStatus_Success)
    {
        /* Compilation info request failed */
        report->Errorf("failed to retrieve shader compilation information\n");
        return;
    }

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
                "%.*s:%u:%u info: %.*s\n%.*s\n%s\n",
                static_cast<int>(sourceContext->sourceName.size()),
                sourceContext->sourceName.data(),
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
                "%.*s:%u:%u %s: %.*s\n%.*s\n%s\n",
                static_cast<int>(sourceContext->sourceName.size()),
                sourceContext->sourceName.data(),
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


/*
 * ======= Private: =======
 */

void WGShaderModule::BuildShader(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext)
{
    const bool isInputLanguageWGSL = (sourceContext.inputLanguage == ShadingLanguage::WGSL);

    /* Setup WebGPU shader source descriptor */
    WGPUShaderSourceWGSL sourceWGSL;
    WGPUShaderSourceSPIRV sourceSPIRV;

    if (isInputLanguageWGSL)
    {
        sourceWGSL.chain        = { nullptr, WGPUSType_ShaderSourceWGSL };
        sourceWGSL.code.data    = sourceContext.sourceText.data();
        sourceWGSL.code.length  = sourceContext.sourceText.size();
    }
    else
    {
        sourceSPIRV.chain       = { nullptr, WGPUSType_ShaderSourceSPIRV };
        sourceSPIRV.codeSize    = static_cast<std::uint32_t>(sourceContext.sourceText.size());
        sourceSPIRV.code        = reinterpret_cast<const std::uint32_t*>(sourceContext.sourceText.data());
    }

    /* Create WebGPU shader module */
    WGPUShaderModuleDescriptor moduleDesc;
    {
        if (isInputLanguageWGSL)
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
        compilationInfoCallback.userdata2   = const_cast<ShaderSourceContext*>(&sourceContext); // Do not modify in the callback
    }
    WGPUFutureWaitInfo waitInfo;
    {
        waitInfo.future     = wgpuShaderModuleGetCompilationInfo(shaderModule_, compilationInfoCallback);
        waitInfo.completed  = WGPU_FALSE;
    }
    const WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT64_MAX);

    if (waitStatus != WGPUWaitStatus_Success)
        report_.Errorf("failed to request WebGPU shader compilation result (%s)", ToString(waitStatus));
}


} // /namespace LLGL



// ================================================================================
