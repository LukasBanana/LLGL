/*
 * MTCommand.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_H
#define LLGL_MT_COMMAND_H


#import <MetalKit/MetalKit.h>

#include <LLGL/CommandBufferFlags.h>
#include <cstdint>


namespace LLGL
{


class MTMultiSubmitCommandBuffer;
class MTGraphicsPSO;
class MTComputePSO;
class MTRenderPass;
class RenderTarget;

struct MTCmdExecute
{
    MTMultiSubmitCommandBuffer* commandBuffer;
};

struct MTCmdCopyBuffer
{
    id<MTLBuffer>   sourceBuffer;
    NSUInteger      sourceOffset;
    id<MTLBuffer>   destinationBuffer;
    NSUInteger      destinationOffset;
    NSUInteger      size;
};

struct MTCmdCopyBufferFromTexture
{
    id<MTLTexture>  sourceTexture;
    NSUInteger      sourceSlice;
    NSUInteger      sourceLevel;
    MTLOrigin       sourceOrigin;
    MTLSize         sourceSize;
    id<MTLBuffer>   destinationBuffer;
    NSUInteger      destinationOffset;
    NSUInteger      destinationBytesPerRow;
    NSUInteger      destinationBytesPerImage;
    NSUInteger      layerCount;
};

struct MTCmdCopyTexture
{
    id<MTLTexture>  sourceTexture;
    NSUInteger      sourceSlice;
    NSUInteger      sourceLevel;
    MTLOrigin       sourceOrigin;
    MTLSize         sourceSize;
    id<MTLTexture>  destinationTexture;
    NSUInteger      destinationSlice;
    NSUInteger      destinationLevel;
    MTLOrigin       destinationOrigin;
};

struct MTCmdCopyTextureFromBuffer
{
    id<MTLBuffer>   sourceBuffer;
    NSUInteger      sourceOffset;
    NSUInteger      sourceBytesPerRow;
    NSUInteger      sourceBytesPerImage;
    MTLSize         sourceSize;
    id<MTLTexture>  destinationTexture;
    NSUInteger      destinationSlice;
    NSUInteger      destinationLevel;
    MTLOrigin       destinationOrigin;
    NSUInteger      layerCount;
};

struct MTCmdCopyTextureFromFramebuffer
{
    MTKView*        sourceView;
    MTLOrigin       sourceOrigin;
    MTLSize         sourceSize;
    id<MTLTexture>  destinationTexture;
    NSUInteger      destinationSlice;
    NSUInteger      destinationLevel;
    MTLOrigin       destinationOrigin;
};

//struct MTCmdPauseRenderEncoder;
//struct MTCmdResumeRenderEncoder;

struct MTCmdGenerateMipmaps
{
    id<MTLTexture> texture;
};

struct MTCmdSetGraphicsPSO
{
    MTGraphicsPSO* graphicsPSO;
};

struct MTCmdSetComputePSO
{
    MTComputePSO* computePSO;
};

struct MTCmdSetTessellationPSO
{
    id<MTLComputePipelineState> tessPipelineState;
    id<MTLBuffer>               tessFactorBuffer;
    NSUInteger                  tessFactorBufferSlot;
};

struct MTCmdSetTessellationFactorBuffer
{
    id<MTLBuffer>   tessFactorBuffer;
    NSUInteger      instanceStride;
};

struct MTCmdSetViewports
{
    NSUInteger  count;
//  Viewport    viewports[count];
};

struct MTCmdSetScissorRects
{
    NSUInteger  count;
//  Scissor     scissorRects[count];
};

struct MTCmdSetBlendColor
{
    float blendColor[4];
};

struct MTCmdSetStencilRef
{
    std::uint32_t   ref;
    StencilFace     face;
};

struct MTCmdSetVertexBuffers
{
    NSUInteger      count;
//  id<MTLBuffer>*  buffers[count];
//  NSUInteger      offsets[count];
};

struct MTCmdSetResourceHeap
{
    MTResourceHeap* resourceHeap;
    std::uint32_t   descriptorSet;
};

struct MTCmdBindRenderTarget
{
    RenderTarget*       renderTarget;
    const MTRenderPass* renderPass;
    std::uint32_t       numClearValues;
//  ClearValue          clearValues[numClearValues];
};

struct MTCmdClearRenderPass
{
    long            flags;
    double          clearDepth;
    std::uint32_t   clearStencil;
    std::uint32_t   numAttachments;
    std::uint32_t   numColorAttachments;
//  std::uint32_t   colorBuffers[numAttachments];   // Not numColorAttachments to simplify allocation
//  MTLClearColor   clearColors[numAttachments];    // Not numColorAttachments to simplify allocation
};

struct MTCmdDrawPatches
{
    NSUInteger controlPointCount;
    NSUInteger patchStart;
    NSUInteger patchCount;
    NSUInteger instanceCount;
    NSUInteger baseInstance;
};

struct MTCmdDrawPrimitives
{
    MTLPrimitiveType    primitiveType;
    NSUInteger          vertexStart;
    NSUInteger          vertexCount;
    NSUInteger          instanceCount;
    NSUInteger          baseInstance;
};

struct MTCmdDrawIndexedPatches
{
    NSUInteger      controlPointCount;
    NSUInteger      patchStart;
    NSUInteger      patchCount;
    id<MTLBuffer>   controlPointIndexBuffer;
    NSUInteger      controlPointIndexBufferOffset;
    NSUInteger      instanceCount;
    NSUInteger      baseInstance;
};

struct MTCmdDrawIndexedPrimitives
{
    MTLPrimitiveType    primitiveType;
    NSUInteger          indexCount;
    MTLIndexType        indexType;
    id<MTLBuffer>       indexBuffer;
    NSUInteger          indexBufferOffset;
    NSUInteger          instanceCount;
    NSUInteger          baseVertex;
    NSUInteger          baseInstance;
};

struct MTCmdDispatchThreads
{
    MTLSize threads;
    MTLSize threadsPerThreadgroup;
};

struct MTCmdDispatchThreadgroups
{
    MTLSize threadgroups;
    MTLSize threadsPerThreadgroup;
};

struct MTCmdDispatchThreadgroupsIndirect
{
    id<MTLBuffer>   indirectBuffer;
    NSUInteger      indirectBufferOffset;
    MTLSize         threadsPerThreadgroup;
};

struct MTCmdPushDebugGroup
{
    NSUInteger  length;
//  char        name[length + 1];
};

//struct MTCmdPopDebugGroup;

struct MTCmdPresentDrawables
{
    NSUInteger  count;
//  MTKView*    views[count];
};

//struct MTCmdFlush;


} // /namespace LLGL


#endif



// ================================================================================
