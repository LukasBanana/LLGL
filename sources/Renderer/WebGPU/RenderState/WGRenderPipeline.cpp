/*
 * WGRenderPipeline.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderPipeline.h"
#include "WGRenderPass.h"
#include "../WGCore.h"
#include "../WGTypes.h"
#include "../Shader/WGShader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>


namespace LLGL
{


static constexpr bool IsWGColorWriteMaskCompatible()
{
    return
    (
        (LLGL::ColorMaskFlags::R == WGPUColorWriteMask_Red  ) &&
        (LLGL::ColorMaskFlags::G == WGPUColorWriteMask_Green) &&
        (LLGL::ColorMaskFlags::B == WGPUColorWriteMask_Blue ) &&
        (LLGL::ColorMaskFlags::A == WGPUColorWriteMask_Alpha)
    );
}

static WGPUColorWriteMask ToWGColorWriteMask(std::uint8_t colorMask)
{
    if (IsWGColorWriteMaskCompatible())
    {
        /* Static cast since both bitmasks are compatible */
        return static_cast<WGPUColorWriteMask>(colorMask);
    }
    else
    {
        /* Map each mask individually otherwise */
        WGPUColorWriteMask outMask = 0;
        if ((colorMask & LLGL::ColorMaskFlags::R) != 0) outMask |= WGPUColorWriteMask_Red;
        if ((colorMask & LLGL::ColorMaskFlags::G) != 0) outMask |= WGPUColorWriteMask_Green;
        if ((colorMask & LLGL::ColorMaskFlags::B) != 0) outMask |= WGPUColorWriteMask_Blue;
        if ((colorMask & LLGL::ColorMaskFlags::A) != 0) outMask |= WGPUColorWriteMask_Alpha;
        return outMask;
    }
}

static void ConvertBlendState(WGPUBlendState& dst, const BlendTargetDescriptor& src)
{
    if (src.blendEnabled)
    {
        dst.color.operation = WGTypes::ToWGBlendOperation(src.colorArithmetic);
        dst.color.srcFactor = WGTypes::ToWGBlendFactor(src.srcColor);
        dst.color.dstFactor = WGTypes::ToWGBlendFactor(src.dstColor);
        dst.alpha.operation = WGTypes::ToWGBlendOperation(src.alphaArithmetic);
        dst.alpha.srcFactor = WGTypes::ToWGBlendFactor(src.srcAlpha);
        dst.alpha.dstFactor = WGTypes::ToWGBlendFactor(src.dstAlpha);
    }
    else
    {
        dst.color.operation = WGPUBlendOperation_Add;
        dst.color.srcFactor = WGPUBlendFactor_One;
        dst.color.dstFactor = WGPUBlendFactor_Zero;
        dst.alpha.operation = WGPUBlendOperation_Add;
        dst.alpha.srcFactor = WGPUBlendFactor_One;
        dst.alpha.dstFactor = WGPUBlendFactor_Zero;
    }
}

static void ConvertStencilFaceState(WGPUStencilFaceState& dst, const StencilFaceDescriptor& src)
{
    dst.compare     = WGTypes::ToWGCompareFunc(src.compareOp);
    dst.failOp      = WGTypes::ToWGStencilOperation(src.stencilFailOp);
    dst.depthFailOp = WGTypes::ToWGStencilOperation(src.depthFailOp);
    dst.passOp      = WGTypes::ToWGStencilOperation(src.depthPassOp);
}

static void ConvertDepthStencilState(WGPUDepthStencilState& dst, const DepthBiasDescriptor& srcDepthBias, const DepthDescriptor& srcDepth, const StencilDescriptor& srcStencil)
{
    dst.nextInChain         = nullptr;
    dst.format              = WGPUTextureFormat_Undefined;
    dst.depthWriteEnabled   = (srcDepth.writeEnabled ? WGPUOptionalBool_True : WGPUOptionalBool_False);
    dst.depthCompare        = WGTypes::ToWGCompareFunc(srcDepth.compareOp);
    ConvertStencilFaceState(dst.stencilFront, srcStencil.front);
    ConvertStencilFaceState(dst.stencilBack, srcStencil.back);
    dst.stencilReadMask     = srcStencil.front.readMask;
    dst.stencilWriteMask    = srcStencil.front.writeMask;
    dst.depthBias           = static_cast<std::int32_t>(srcDepthBias.constantFactor);
    dst.depthBiasSlopeScale = srcDepthBias.slopeFactor;
    dst.depthBiasClamp      = srcDepthBias.clamp;
}

WGRenderPipeline::WGRenderPipeline(WGPUDevice device, const GraphicsPipelineDescriptor& desc) :
    WGPipelineState { /*isRenderPipeline:*/ true }
{
    /* Get number of render-target attachments */
    const WGRenderPass* renderPassWG = (desc.renderPass != nullptr ? LLGL_CAST(const WGRenderPass*, desc.renderPass) : nullptr);

    const WGShader* vertexShaderWG = LLGL_CAST(const WGShader*, desc.vertexShader);
    if (vertexShaderWG == nullptr)
    {
        GetMutableReport().Errorf("cannot create WebGPU render pipeline without vertex shader\n");
        return;
    }

    WGPUBlendState blendTargetStates[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    WGPUColorTargetState colorTargetStates[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    WGPUFragmentState fragmentState;

    if (desc.fragmentShader != nullptr)
    {
        const WGShader* fragmentShaderWG = LLGL_CAST(const WGShader*, desc.fragmentShader);

        const std::size_t numColorTargetStates = (renderPassWG != nullptr ? renderPassWG->GetColorTargetFormats().size() : 1u);
        LLGL_ASSERT(numColorTargetStates <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);

        const std::size_t numBlendTargets = (desc.blend.independentBlendEnabled ? numColorTargetStates : 1u);
        for_range(i, numBlendTargets)
            ConvertBlendState(blendTargetStates[i], desc.blend.targets[i]);

        for_range(i, numColorTargetStates)
        {
            colorTargetStates[i].nextInChain    = nullptr;
            colorTargetStates[i].format         = (renderPassWG != nullptr ? renderPassWG->GetColorTargetFormats()[i] : WGPUTextureFormat_BGRA8Unorm);
            colorTargetStates[i].blend          = &(blendTargetStates[desc.blend.independentBlendEnabled ? i : 0]);
            colorTargetStates[i].writeMask      = ToWGColorWriteMask(desc.blend.targets[i].colorMask);
        }

        fragmentState.nextInChain   = nullptr;
        fragmentState.module        = fragmentShaderWG->GetNative();
        fragmentState.entryPoint    = fragmentShaderWG->GetEntryPointNameView();
        fragmentState.constantCount = 0;
        fragmentState.constants     = nullptr;
        fragmentState.targetCount   = numColorTargetStates;
        fragmentState.targets       = colorTargetStates;
    }

    WGPUDepthStencilState depthStencilState;
    ConvertDepthStencilState(depthStencilState, desc.rasterizer.depthBias, desc.depth, desc.stencil);

    WGPURenderPipelineDescriptor renderPipelineDesc;
    {
        renderPipelineDesc.nextInChain  = nullptr;
        renderPipelineDesc.label        = WGPU_STRING_VIEW_INIT;
        renderPipelineDesc.layout       = nullptr; //TODO

        renderPipelineDesc.vertex.nextInChain   = nullptr;
        renderPipelineDesc.vertex.module        = vertexShaderWG->GetNative();
        renderPipelineDesc.vertex.entryPoint    = vertexShaderWG->GetEntryPointNameView();
        renderPipelineDesc.vertex.constantCount = 0;
        renderPipelineDesc.vertex.constants     = nullptr;

        if (const WGVertexInputLayout* vertexInputLayout = vertexShaderWG->GetVertexInputLayout())
        {
            renderPipelineDesc.vertex.bufferCount   = vertexInputLayout->GetVertexBufferLayouts().size();
            renderPipelineDesc.vertex.buffers       = vertexInputLayout->GetVertexBufferLayouts().data();
        }
        else
        {
            renderPipelineDesc.vertex.bufferCount   = 0;
            renderPipelineDesc.vertex.buffers       = nullptr;
        }

        renderPipelineDesc.primitive.nextInChain        = nullptr;
        renderPipelineDesc.primitive.topology           = WGTypes::ToWGPrimitiveTopology(desc.primitiveTopology);
        renderPipelineDesc.primitive.stripIndexFormat   = WGTypes::ToWGIndexFormat(desc.indexFormat);
        renderPipelineDesc.primitive.frontFace          = (desc.rasterizer.frontCCW ? WGPUFrontFace_CCW : WGPUFrontFace_CW);
        renderPipelineDesc.primitive.cullMode           = WGTypes::ToWGCullMode(desc.rasterizer.cullMode);
        renderPipelineDesc.primitive.unclippedDepth     = (desc.rasterizer.depthClampEnabled ? WGPU_TRUE : WGPU_FALSE);

        renderPipelineDesc.depthStencil = &depthStencilState;

        renderPipelineDesc.multisample.nextInChain              = nullptr;
        renderPipelineDesc.multisample.count                    = 1; //TODO
        renderPipelineDesc.multisample.mask                     = desc.blend.sampleMask;
        renderPipelineDesc.multisample.alphaToCoverageEnabled   = (desc.blend.alphaToCoverageEnabled ? WGPU_TRUE : WGPU_FALSE);

        renderPipelineDesc.fragment     = (desc.rasterizer.discardEnabled ? nullptr : &fragmentState);
    }
    renderPipeline_ = wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
}


} // /namespace LLGL



// ================================================================================
