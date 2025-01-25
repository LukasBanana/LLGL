/*
 * MTGraphicsPSO.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTGraphicsPSO.h"
#include "MTRenderPass.h"
#include "MTPipelineLayout.h"
#include "../Shader/MTShader.h"
//#include "../Command/MTCommandContext.h"
#include "../MTTypes.h"
#include "../MTCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/ByteBufferIterator.h"
#include "../../../Core/Assertion.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/ForRange.h>
#include <TargetConditionals.h>
#include <string.h>


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
    MTPipelineState { /*isGraphicsPSO:*/ true, desc.pipelineLayout }
{
    /* Convert standalone parameters */
    cullMode_       	= MTTypes::ToMTLCullMode(desc.rasterizer.cullMode);
    winding_            = (desc.rasterizer.frontCCW ? MTLWindingCounterClockwise : MTLWindingClockwise);
    fillMode_           = MTTypes::ToMTLTriangleFillMode(desc.rasterizer.polygonMode);
    primitiveType_  	= MTTypes::ToMTLPrimitiveType(desc.primitiveTopology);
    clipMode_           = (desc.rasterizer.depthClampEnabled ? MTLDepthClipModeClamp : MTLDepthClipModeClip);

    depthBias_          = desc.rasterizer.depthBias.constantFactor;
    depthSlope_         = desc.rasterizer.depthBias.slopeFactor;
    depthClamp_         = desc.rasterizer.depthBias.clamp;

    hasScissorTest_     = desc.rasterizer.scissorTestEnabled;

    blendColorDynamic_  = desc.blend.blendFactorDynamic;
    blendColorEnabled_  = IsStaticBlendFactorEnabled(desc.blend);
    blendColor_[0]      = desc.blend.blendFactor[0];
    blendColor_[1]      = desc.blend.blendFactor[1];
    blendColor_[2]      = desc.blend.blendFactor[2];
    blendColor_[3]      = desc.blend.blendFactor[3];

    /* Create render pipeline and depth-stencil states */
    if (CreateRenderPipelineState(device, desc, defaultRenderPass))
    {
        CreateDepthStencilState(device, desc);
        BuildStaticStateBuffer(desc);
    }
}

void MTGraphicsPSO::Bind(id<MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder setRenderPipelineState:renderPipelineState_];
    [renderEncoder setDepthStencilState:depthStencilState_];
    [renderEncoder setCullMode:cullMode_];
    [renderEncoder setFrontFacingWinding:winding_];
    [renderEncoder setTriangleFillMode:fillMode_];

    #if TARGET_OS_SIMULATOR == 0
    if (@available(macOS 10.11, iOS 11, *))
        [renderEncoder setDepthClipMode:clipMode_];
    #endif

    #ifndef LLGL_OS_IOS//TODO: disabled for testing iOS
    [renderEncoder setDepthBias:depthBias_ slopeScale:depthSlope_ clamp:depthClamp_];

    if (!stencilRefDynamic_)
    {
        if (stencilFrontRef_ != stencilBackRef_)
            [renderEncoder setStencilFrontReferenceValue:stencilFrontRef_ backReferenceValue:stencilBackRef_];
        else
            [renderEncoder setStencilReferenceValue:stencilFrontRef_];
    }
    #endif

    if (blendColorEnabled_)
    {
        [renderEncoder
            setBlendColorRed:   blendColor_[0]
            green:              blendColor_[1]
            blue:               blendColor_[2]
            alpha:              blendColor_[3]
        ];
    }

    if (const MTPipelineLayout* pipelineLayout = GetPipelineLayout())
    {
        pipelineLayout->SetStaticVertexSamplers(renderEncoder);
        pipelineLayout->SetStaticFragmentSamplers(renderEncoder);
    }
}

bool MTGraphicsPSO::GetStaticState(
    MTLViewport*    outViewports,
    NSUInteger&     outViewportCount,
    MTLScissorRect* outScissorRects,
    NSUInteger&     outScissorRectCount) const
{
    if (staticStateBuffer_)
    {
        ByteBufferConstIterator byteBufferIter{ staticStateBuffer_.get() };
        ::memcpy(outViewports, byteBufferIter.Next<MTLViewport>(numStaticViewports_), numStaticViewports_ * sizeof(MTLViewport));
        outViewportCount = numStaticViewports_;
        ::memcpy(outScissorRects, byteBufferIter.Next<MTLScissorRect>(numStaticScissors_), numStaticScissors_ * sizeof(MTLScissorRect));
        outScissorRectCount = numStaticScissors_;
    }
    return false;
}


/*
 * ======= Private: =======
 */

static MTLColorWriteMask ToMTLColorWriteMask(std::uint8_t colorMask)
{
    MTLColorWriteMask writeMask = MTLColorWriteMaskNone;

    if ((colorMask & ColorMaskFlags::R) != 0) { writeMask |= MTLColorWriteMaskRed;   }
    if ((colorMask & ColorMaskFlags::G) != 0) { writeMask |= MTLColorWriteMaskGreen; }
    if ((colorMask & ColorMaskFlags::B) != 0) { writeMask |= MTLColorWriteMaskBlue;  }
    if ((colorMask & ColorMaskFlags::A) != 0) { writeMask |= MTLColorWriteMaskAlpha; }

    return writeMask;
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

static id<MTLFunction> GetNativeMTShader(const Shader* shader)
{
    return (shader != nullptr ? LLGL_CAST(const MTShader*, shader)->GetNative() : nil);
}

static const MTShader* GetVertexOrPostTessVertexShader(const GraphicsPipelineDescriptor& desc)
{
    /* For tessellation PSOs, the vertex shader must be a post-tesselation vertex shader and is expected in the tessEvaluationShader field */
    const bool hasTessellationStage = (desc.tessControlShader != nullptr && desc.tessControlShader->GetType() == ShaderType::Compute);
    const MTShader* vertexShaderMT = nullptr;
    if (hasTessellationStage)
    {
        vertexShaderMT = LLGL_CAST(const MTShader*, desc.tessEvaluationShader);

        LLGL_ASSERT(
            vertexShaderMT != nullptr && vertexShaderMT->IsPostTessellationVertex(),
            "cannot create Metal tessellation pipeline without post-tessellation vertex shader (in 'tessEvaluationShader')"
        );
    }
    else
    {
        vertexShaderMT = LLGL_CAST(const MTShader*, desc.vertexShader);

        LLGL_ASSERT(vertexShaderMT != nullptr, "cannot create Metal pipeline without vertex shader");
    }
    return vertexShaderMT;
}

bool MTGraphicsPSO::CreateRenderPipelineState(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc,
    const MTRenderPass*                 defaultRenderPass)
{
    /* Get native shader functions */
    const MTShader* vertexShaderMT = GetVertexOrPostTessVertexShader(desc);

    /* Get number of patch control points if a post-tessellation vertex function is specified */
    numPatchControlPoints_ = vertexShaderMT->GetNumPatchControlPoints();

    if (id<MTLFunction> vertexFunc = vertexShaderMT->GetNative())
    {
        /* Cache patch type of vertex function */
        patchType_ = [vertexFunc patchType];
    }
    else
    {
        /* Error: Every graphics PSO needs a valid vertex function */
        GetMutableReport().Errorf("cannot create Metal graphics PSO without valid vertex function");
        return false;
    }

    /* Get render pass object */
    const MTRenderPass* renderPassMT = nullptr;
    if (const RenderPass* renderPass = desc.renderPass)
        renderPassMT = LLGL_CAST(const MTRenderPass*, renderPass);
    else if (defaultRenderPass != nullptr)
        renderPassMT = defaultRenderPass;
    else
        LLGL_TRAP("cannot create graphics pipeline without render pass");

    /* Create render pipeline state */
    MTLRenderPipelineDescriptor* psoDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        psoDesc.vertexDescriptor        = vertexShaderMT->GetMTLVertexDesc();
        psoDesc.alphaToCoverageEnabled  = MTBoolean(desc.blend.alphaToCoverageEnabled);
        psoDesc.alphaToOneEnabled       = NO;
        psoDesc.fragmentFunction        = GetNativeMTShader(desc.fragmentShader);
        psoDesc.vertexFunction          = vertexShaderMT->GetNative();

        if (@available(iOS 12.0, *))
            psoDesc.inputPrimitiveTopology = MTTypes::ToMTLPrimitiveTopologyClass(desc.primitiveTopology);

        /* Initialize pixel formats from render pass */
        const MTColorAttachmentFormatVector& colorAttachments = renderPassMT->GetColorAttachments();
        for_range(i, std::min(colorAttachments.size(), std::size_t(LLGL_MAX_NUM_COLOR_ATTACHMENTS)))
        {
            FillColorAttachmentDesc(
                psoDesc.colorAttachments[i],
                colorAttachments[i].pixelFormat,
                desc.blend,
                desc.blend.targets[desc.blend.independentBlendEnabled ? i : 0]
            );
        };

        psoDesc.depthAttachmentPixelFormat      = renderPassMT->GetDepthAttachment().pixelFormat;
        psoDesc.stencilAttachmentPixelFormat    = renderPassMT->GetStencilAttachment().pixelFormat;
        psoDesc.rasterizationEnabled            = (desc.rasterizer.discardEnabled ? NO : YES);
        psoDesc.rasterSampleCount               = (desc.rasterizer.multiSampleEnabled ? renderPassMT->GetSampleCount() : 1u);

        /* Specify tessellation state */
        if (numPatchControlPoints_ > 0)
        {
            #ifdef LLGL_OS_IOS
            psoDesc.maxTessellationFactor               = std::min(desc.tessellation.maxTessFactor, 16u);
            #else
            psoDesc.maxTessellationFactor               = std::min(desc.tessellation.maxTessFactor, 64u);
            #endif
            psoDesc.tessellationFactorScaleEnabled      = NO;
            psoDesc.tessellationFactorFormat            = MTLTessellationFactorFormatHalf; // Can only be <half>
            psoDesc.tessellationControlPointIndexType   = MTTypes::ToMTLPatchIndexType(desc.indexFormat);
            psoDesc.tessellationFactorStepFunction      = MTLTessellationFactorStepFunctionPerPatchAndPerInstance; // Same behavior as in D3D
            psoDesc.tessellationOutputWindingOrder      = (desc.tessellation.outputWindingCCW ? MTLWindingCounterClockwise : MTLWindingClockwise);
            psoDesc.tessellationPartitionMode           = MTTypes::ToMTLPartitionMode(desc.tessellation.partition);
        }
    }
    NSError* error = nullptr;
    renderPipelineState_ = CreateNativeRenderPipelineState(device, psoDesc, error);
    if (!renderPipelineState_)
        MTThrowIfCreateFailed(error, "MTLRenderPipelineState");
    [psoDesc release];

    /* Create compute PSO for tessellation stage */
    if (numPatchControlPoints_ > 0)
    {
        if (auto* tessComputeShaderMT = LLGL_CAST(const MTShader*, desc.tessControlShader))
        {
            tessPipelineState_ = [device newComputePipelineStateWithFunction:tessComputeShaderMT->GetNative() error:&error];
            if (!tessPipelineState_)
                MTThrowIfCreateFailed(error, "MTLComputePipelineState");
        }
        else
            LLGL_TRAP("cannot create Metal tessellation pipeline without tessellation compute shader");
    }

    return true;
}

id<MTLRenderPipelineState> MTGraphicsPSO::CreateNativeRenderPipelineState(
    id<MTLDevice>                   device,
    MTLRenderPipelineDescriptor*    desc,
    NSError*&                       error)
{
    if (NeedsConstantsCache())
    {
        /* Create PSO with reflection to generate constants cache */
        MTLAutoreleasedRenderPipelineReflection reflection = nil;
        id<MTLRenderPipelineState> pso = [device
            newRenderPipelineStateWithDescriptor:   desc
            options:                                (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo)
            reflection:                             &reflection
            error:                                  &error
        ];
        CreateConstantsCacheForRenderPipeline(reflection);
        return pso;
    }
    else
        return [device newRenderPipelineStateWithDescriptor:desc error:&error];
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
        stencilRefDynamic_  = desc.stencil.referenceDynamic;
    }
    depthStencilState_ = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
    [depthStencilDesc release];
}

void MTGraphicsPSO::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    /* Allocate packed raw buffer */
    const std::size_t bufferSize =
    (
        desc.viewports.size() * sizeof(MTLViewport) +
        desc.scissors.size()  * sizeof(MTLScissorRect)
    );
    staticStateBuffer_ = DynamicByteArray{ bufferSize, UninitializeTag{} };

    ByteBufferIterator byteBufferIter{ staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!desc.viewports.empty())
        BuildStaticViewports(desc.viewports.size(), desc.viewports.data(), byteBufferIter);

    /* Build static scissors in raw buffer */
    if (!desc.scissors.empty())
        BuildStaticScissors(desc.scissors.size(), desc.scissors.data(), byteBufferIter);
}

void MTGraphicsPSO::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
{
    /* Store number of viewports and validate limit */
    numStaticViewports_ = static_cast<NSUInteger>(numViewports);

    if (numStaticViewports_ > LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS)
    {
        GetMutableReport().Errorf(
            "too many viewports in graphics pipeline state (%d specified, but limit is %u)",
            numStaticViewports_, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS
        );
        numStaticViewports_ = LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS;
    }

    /* Build <MTLViewport> entries */
    for_range(i, numViewports)
    {
        auto dst = byteBufferIter.Next<MTLViewport>();
        {
            dst->originX    = viewports[i].x;
            dst->originY    = viewports[i].y;
            dst->width      = viewports[i].width;
            dst->height     = viewports[i].height;
            dst->znear      = viewports[i].minDepth;
            dst->zfar       = viewports[i].maxDepth;
        }
    }
}

void MTGraphicsPSO::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
{
    /* Store number of scissors and validate limit */
    numStaticScissors_ = static_cast<NSUInteger>(numScissors);

    if (numStaticScissors_ > LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS)
    {
        GetMutableReport().Errorf(
            "too many scissors in graphics pipeline state (%d specified, but limit is %u)",
            numStaticScissors_, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS
        );
        numStaticScissors_ = LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS;
    }

    /* Build <MTLScissorRect> entries */
    for_range(i, numScissors)
    {
        auto dst = byteBufferIter.Next<MTLScissorRect>();
        {
            dst->x      = static_cast<NSUInteger>(scissors[i].x);
            dst->y      = static_cast<NSUInteger>(scissors[i].y);
            dst->width  = static_cast<NSUInteger>(scissors[i].width);
            dst->height = static_cast<NSUInteger>(scissors[i].height);
        }
    }
}


} // /namespace LLGL



// ================================================================================
