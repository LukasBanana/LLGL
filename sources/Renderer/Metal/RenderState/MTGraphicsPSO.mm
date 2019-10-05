/*
 * MTGraphicsPSO.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTGraphicsPSO.h"
#include "MTRenderPass.h"
#include "../Shader/MTShaderProgram.h"
#include "../MTEncoderScheduler.h"
#include "../MTTypes.h"
#include "../MTCore.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


static void Convert(MTLStencilDescriptor* dst, const StencilFaceDescriptor& src)
{
    dst.stencilFailureOperation     = MTTypes::ToMTLStencilOperation(src.stencilFailOp);
    dst.depthFailureOperation       = MTTypes::ToMTLStencilOperation(src.depthFailOp);
    dst.depthStencilPassOperation   = MTTypes::ToMTLStencilOperation(src.depthPassOp);
    dst.stencilCompareFunction      = MTTypes::ToMTLCompareFunction(src.compareOp);
    dst.readMask                    = src.readMask;
    dst.writeMask                   = src.writeMask;
}

static void FillDefaultMTStencilDesc(MTLStencilDescriptor* dst)
{
    dst.stencilFailureOperation     = MTLStencilOperationKeep;
    dst.depthFailureOperation       = MTLStencilOperationKeep;
    dst.depthStencilPassOperation   = MTLStencilOperationKeep;
    dst.stencilCompareFunction      = MTLCompareFunctionAlways;
    dst.readMask                    = 0;
    dst.writeMask                   = 0;
}

MTGraphicsPSO::MTGraphicsPSO(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc,
    const MTRenderPass*                 defaultRenderPass)
:
    MTPipelineState { true }
{
    /* Convert standalone parameters */
    cullMode_       = MTTypes::ToMTLCullMode(desc.rasterizer.cullMode);
    winding_        = (desc.rasterizer.frontCCW ? MTLWindingCounterClockwise : MTLWindingClockwise);
    fillMode_       = MTTypes::ToMTLTriangleFillMode(desc.rasterizer.polygonMode);
    primitiveType_  = MTTypes::ToMTLPrimitiveType(desc.primitiveTopology);
    clipMode_       = (desc.rasterizer.depthClampEnabled ? MTLDepthClipModeClamp : MTLDepthClipModeClip);
    depthBias_      = desc.rasterizer.depthBias.constantFactor;
    depthSlope_     = desc.rasterizer.depthBias.slopeFactor;
    depthClamp_     = desc.rasterizer.depthBias.clamp;

    /* Create render pipeline and depth-stencil states */
    CreateRenderPipelineState(device, desc, defaultRenderPass);
    CreateDepthStencilState(device, desc);
}

void MTGraphicsPSO::Bind(id<MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder setRenderPipelineState:renderPipelineState_];
    [renderEncoder setDepthStencilState:depthStencilState_];
    [renderEncoder setCullMode:cullMode_];
    [renderEncoder setFrontFacingWinding:winding_];
    [renderEncoder setTriangleFillMode:fillMode_];
    #ifndef LLGL_OS_IOS//TODO: since MTLFeatureSet_iOS_GPUFamily2_v1
    [renderEncoder setDepthClipMode:clipMode_];
    #endif
    #ifndef LLGL_OS_IOS//TODO: disabled for testing iOS
    [renderEncoder setDepthBias:depthBias_ slopeScale:depthSlope_ clamp:depthClamp_];
    if (stencilFrontRef_ == stencilBackRef_)
        [renderEncoder setStencilFrontReferenceValue:stencilFrontRef_ backReferenceValue:stencilBackRef_];
    else
        [renderEncoder setStencilReferenceValue:stencilFrontRef_];
    #endif
}


/*
 * ======= Private: =======
 */

static MTLColorWriteMask ToMTLColorWriteMask(const ColorRGBAb& color)
{
    MTLColorWriteMask mask = MTLColorWriteMaskNone;

    if (color.r)
        mask |= MTLColorWriteMaskRed;
    if (color.g)
        mask |= MTLColorWriteMaskGreen;
    if (color.b)
        mask |= MTLColorWriteMaskBlue;
    if (color.a)
        mask |= MTLColorWriteMaskAlpha;

    return mask;
}

static void FillColorAttachmentDesc(
    MTLRenderPipelineColorAttachmentDescriptor* dst,
    MTLPixelFormat                              pixelFormat,
    const BlendDescriptor&                      blendDesc,
    const BlendTargetDescriptor&                targetDesc)
{
    /* Render pipeline state */
    dst.pixelFormat                 = pixelFormat;
    dst.writeMask                   = ToMTLColorWriteMask(targetDesc.colorMask);

    /* Controlling blend operation */
    dst.blendingEnabled             = (targetDesc.blendEnabled ? YES : NO);
    dst.alphaBlendOperation         = MTTypes::ToMTLBlendOperation(targetDesc.alphaArithmetic);
    dst.rgbBlendOperation           = MTTypes::ToMTLBlendOperation(targetDesc.colorArithmetic);

    /* Blend factors */
    dst.destinationAlphaBlendFactor = MTTypes::ToMTLBlendFactor(targetDesc.dstAlpha);
    dst.destinationRGBBlendFactor   = MTTypes::ToMTLBlendFactor(targetDesc.dstColor);
    dst.sourceAlphaBlendFactor      = MTTypes::ToMTLBlendFactor(targetDesc.srcAlpha);
    dst.sourceRGBBlendFactor        = MTTypes::ToMTLBlendFactor(targetDesc.srcColor);
}

void MTGraphicsPSO::CreateRenderPipelineState(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc,
    const MTRenderPass*                 defaultRenderPass)
{
    /* Get native shader functions */
    auto shaderProgramMT = LLGL_CAST(const MTShaderProgram*, desc.shaderProgram);
    if (!shaderProgramMT)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Get render pass object */
    const MTRenderPass* renderPassMT = nullptr;
    if (auto renderPass = desc.renderPass)
        renderPassMT = LLGL_CAST(const MTRenderPass*, renderPass);
    else if (defaultRenderPass != nullptr)
        renderPassMT = defaultRenderPass;
    else
        throw std::invalid_argument("cannot create graphics pipeline without render pass");

    /* Create render pipeline state */
    MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        renderPipelineDesc.vertexDescriptor         = shaderProgramMT->GetMTLVertexDesc();
        renderPipelineDesc.alphaToCoverageEnabled   = MTBoolean(desc.blend.alphaToCoverageEnabled);
        renderPipelineDesc.alphaToOneEnabled        = NO;
        renderPipelineDesc.fragmentFunction         = shaderProgramMT->GetFragmentMTLFunction();
        renderPipelineDesc.vertexFunction           = shaderProgramMT->GetVertexMTLFunction();

        if (@available(iOS 12.0, *))
            renderPipelineDesc.inputPrimitiveTopology   = MTTypes::ToMTLPrimitiveTopologyClass(desc.primitiveTopology);

        /* Initialize pixel formats from render pass */
        const auto& colorAttachments = renderPassMT->GetColorAttachments();
        for (std::size_t i = 0, n = std::min(colorAttachments.size(), std::size_t(8u)); i < n; ++i)
        {
            FillColorAttachmentDesc(
                renderPipelineDesc.colorAttachments[i],
                colorAttachments[i].pixelFormat,
                desc.blend,
                desc.blend.targets[desc.blend.independentBlendEnabled ? i : 0]
            );
        };

        renderPipelineDesc.depthAttachmentPixelFormat       = renderPassMT->GetDepthAttachment().pixelFormat;
        renderPipelineDesc.stencilAttachmentPixelFormat     = renderPassMT->GetStencilAttachment().pixelFormat;
        renderPipelineDesc.rasterizationEnabled             = (desc.rasterizer.discardEnabled ? NO : YES);
        renderPipelineDesc.sampleCount                      = (desc.rasterizer.multiSampleEnabled ? renderPassMT->GetSampleCount() : 1u);
    }
    NSError* error = nullptr;
    renderPipelineState_ = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:&error];
    [renderPipelineDesc release];

    if (!renderPipelineState_)
        MTThrowIfCreateFailed(error, "MTLRenderPipelineState");
}

void MTGraphicsPSO::CreateDepthStencilState(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc)
{
    MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
    {
        /* Convert depth descriptor */
        depthStencilDesc.depthWriteEnabled          = MTBoolean(desc.depth.writeEnabled);
        if (desc.depth.testEnabled)
            depthStencilDesc.depthCompareFunction   = MTTypes::ToMTLCompareFunction(desc.depth.compareOp);
        else
            depthStencilDesc.depthCompareFunction   = MTLCompareFunctionAlways;

        /* Convert stencil descriptor */
        if (desc.stencil.testEnabled)
        {
            Convert(depthStencilDesc.frontFaceStencil, desc.stencil.front);
            Convert(depthStencilDesc.backFaceStencil, desc.stencil.back);
        }
        else
        {
            FillDefaultMTStencilDesc(depthStencilDesc.frontFaceStencil);
            FillDefaultMTStencilDesc(depthStencilDesc.backFaceStencil);
        }

        /* Store stencil reference values */
        stencilFrontRef_    = desc.stencil.front.reference;
        stencilBackRef_     = desc.stencil.back.reference;
    }
    depthStencilState_ = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
    [depthStencilDesc release];
}


} // /namespace LLGL



// ================================================================================
