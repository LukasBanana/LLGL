/*
 * MTGraphicsPipeline.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTGraphicsPipeline.h"
#include "../../CheckedCast.h"
#include "../Shader/MTShaderProgram.h"
#include "../MTTypes.h"
#include <LLGL/GraphicsPipelineFlags.h>
#include <string>
#include <stdexcept>


namespace LLGL
{


static void MTThrowIfFailed(NSError* error, const char* info)
{
    if (error != nullptr)
    {
        std::string s = info;
        
        NSString* errorMsg = [error localizedDescription];
        s += [errorMsg cStringUsingEncoding:NSUTF8StringEncoding];
        
        throw std::runtime_error(s);
    }
}

static void MTThrowIfCreateFailed(NSError* error, const char* interfaceName, const char* contextInfo = nullptr)
{
    if (error != nullptr)
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
        MTThrowIfFailed(error, s.c_str());
    }
}

static BOOL MTBoolean(bool value)
{
    return (value ? YES : NO);
}

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

MTGraphicsPipeline::MTGraphicsPipeline(id<MTLDevice> device, const GraphicsPipelineDescriptor& desc)
{
    /* Convert standalone parameters */
    primitiveType_ = MTTypes::ToMTLPrimitiveType(desc.primitiveTopology);

    /* Create render pipeline and depth-stencil states */
    CreateRenderPipelineState(device, desc);
    CreateDepthStencilState(device, desc);
}


/*
 * ======= Private: =======
 */

void MTGraphicsPipeline::CreateRenderPipelineState(id<MTLDevice> device, const GraphicsPipelineDescriptor& desc)
{
    /* Get native shader functions */
    auto shaderProgramMT = LLGL_CAST(const MTShaderProgram*, desc.shaderProgram);
    if (!shaderProgramMT)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");
    
    /* Create render pipeline state */
    MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        renderPipelineDesc.vertexDescriptor         = shaderProgramMT->GetMTLVertexDesc();
        renderPipelineDesc.alphaToCoverageEnabled   = MTBoolean(desc.blend.alphaToCoverageEnabled);
        renderPipelineDesc.alphaToOneEnabled        = NO;
        renderPipelineDesc.fragmentFunction         = shaderProgramMT->GetFragmentMTLFunction();
        renderPipelineDesc.vertexFunction           = shaderProgramMT->GetVertexMTLFunction();
        renderPipelineDesc.inputPrimitiveTopology   = MTTypes::ToMTLPrimitiveTopologyClass(desc.primitiveTopology);
        
        //TODO: get pixel formats from render target or render context
        renderPipelineDesc.colorAttachments[0].pixelFormat  = MTLPixelFormatBGRA8Unorm;
        renderPipelineDesc.depthAttachmentPixelFormat       = MTLPixelFormatDepth32Float_Stencil8;
        renderPipelineDesc.stencilAttachmentPixelFormat     = MTLPixelFormatDepth32Float_Stencil8;
        
        if (shaderProgramMT->GetFragmentMTLFunction() != nil)
        {
            renderPipelineDesc.rasterizationEnabled = YES;
            renderPipelineDesc.sampleCount          = desc.rasterizer.multiSampling.SampleCount();
        }
        else
            renderPipelineDesc.rasterizationEnabled = NO;
    }
    NSError* error = nullptr;
    renderPipelineState_ = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:&error];
    [renderPipelineDesc release];
    
    if (!renderPipelineState_)
        MTThrowIfCreateFailed(error, "MTLRenderPipelineState");
}

void MTGraphicsPipeline::CreateDepthStencilState(id<MTLDevice> device, const GraphicsPipelineDescriptor& desc)
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
        stencilRef_ = desc.stencil.front.reference;
    }
    depthStencilState_ = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
    [depthStencilDesc release];
}


} // /namespace LLGL



// ================================================================================
