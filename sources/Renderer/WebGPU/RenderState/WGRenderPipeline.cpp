/*
 * WGRenderPipeline.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderPipeline.h"
#include "../WGCore.h"
#include "../Shader/WGShader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <string.h>


namespace LLGL
{


WGRenderPipeline::WGRenderPipeline(WGPUDevice device, const GraphicsPipelineDescriptor& desc) :
    WGPipelineState { /*isRenderPipeline:*/ true }
{
    const WGShader* vertexShaderWG = LLGL_CAST(const WGShader*, desc.vertexShader);
    if (vertexShaderWG == nullptr)
    {
        GetMutableReport().Errorf("cannot create WebGPU render pipeline without vertex shader\n");
        return;
    }

    WGPUColorTargetState colorTargetStates[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    colorTargetStates[0].nextInChain    = nullptr;
    colorTargetStates[0].format         = WGPUTextureFormat_BGRA8Unorm;
    colorTargetStates[0].blend          = nullptr;
    colorTargetStates[0].writeMask      = WGPUColorWriteMask_All;

    WGPUFragmentState fragmentState;
    if (desc.fragmentShader != nullptr)
    {
        const WGShader* fragmentShaderWG = LLGL_CAST(const WGShader*, desc.fragmentShader);

        fragmentState.nextInChain   = nullptr;
        fragmentState.module        = fragmentShaderWG->GetNative();
        fragmentState.entryPoint    = ToWGStringView("PS");
        fragmentState.constantCount = 0;
        fragmentState.constants     = nullptr;
        fragmentState.targetCount   = 1;
        fragmentState.targets       = colorTargetStates;
    }

    WGPURenderPipelineDescriptor renderPipelineDesc;
    {
        renderPipelineDesc.nextInChain  = nullptr;
        renderPipelineDesc.label        = WGPU_STRING_VIEW_INIT;
        renderPipelineDesc.layout       = nullptr; //TODO

        renderPipelineDesc.vertex.nextInChain   = nullptr;
        renderPipelineDesc.vertex.module        = vertexShaderWG->GetNative();
        renderPipelineDesc.vertex.entryPoint    = ToWGStringView("VS");
        renderPipelineDesc.vertex.constantCount = 0;
        renderPipelineDesc.vertex.constants     = nullptr;
        renderPipelineDesc.vertex.bufferCount   = 0;
        renderPipelineDesc.vertex.buffers       = nullptr;

        renderPipelineDesc.primitive.nextInChain        = nullptr; //WGPUChainedStruct *
        renderPipelineDesc.primitive.topology           = WGPUPrimitiveTopology_TriangleList; //TODO
        renderPipelineDesc.primitive.stripIndexFormat   = WGPUIndexFormat_Undefined; //TODO
        renderPipelineDesc.primitive.frontFace          = WGPUFrontFace_CCW;
        renderPipelineDesc.primitive.cullMode           = WGPUCullMode_None;
        renderPipelineDesc.primitive.unclippedDepth     = WGPU_FALSE;

        renderPipelineDesc.depthStencil = nullptr;
        /*renderPipelineDesc.depthStencil.nextInChain         = ; //WGPUChainedStruct *
        renderPipelineDesc.depthStencil.format              = ; //WGPUTextureFormat
        renderPipelineDesc.depthStencil.depthWriteEnabled               = ; //WGPUOptionalBool
        renderPipelineDesc.depthStencil.depthCompare                = ; //WGPUCompareFunction
        renderPipelineDesc.depthStencil.stencilFront                = ; //WGPUStencilFaceState
        renderPipelineDesc.depthStencil.stencilBack             = ; //WGPUStencilFaceState
        renderPipelineDesc.depthStencil.stencilReadMask             = ; //uint32_t
        renderPipelineDesc.depthStencil.stencilWriteMask                = ; //uint32_t
        renderPipelineDesc.depthStencil.depthBias               = ; //int32_t
        renderPipelineDesc.depthStencil.depthBiasSlopeScale             = ; //float
        renderPipelineDesc.depthStencil.depthBiasClamp              = ; //float*/

        renderPipelineDesc.multisample.nextInChain              = nullptr;
        renderPipelineDesc.multisample.count                    = 1; //TODO
        renderPipelineDesc.multisample.mask                     = desc.blend.sampleMask;
        renderPipelineDesc.multisample.alphaToCoverageEnabled   = WGPU_FALSE; //WGPUBool

        renderPipelineDesc.fragment     = &fragmentState; //WGPU_NULLABLE WGPUFragmentState const *
    }
    renderPipeline_ = wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
}


} // /namespace LLGL



// ================================================================================
