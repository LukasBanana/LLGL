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

struct MTCmdSetUniforms
{
    std::uint32_t   first;
    std::uint16_t   dataSize;
//  char            data[dataSize];
};

struct MTCmdSetVertexBuffers
{
    NSUInteger      count;
//  id<MTLBuffer>*  buffers[count];
//  NSUInteger      offsets[count];
};

struct MTCmdSetIndexBuffer
{
    id<MTLBuffer>   buffer;
    NSUInteger      offset;
    bool            indexType16Bits;
};

struct MTCmdSetResourceHeap
{
    MTResourceHeap* resourceHeap;
    std::uint32_t   descriptorSet;
};

struct MTCmdSetResource
{
    std::uint32_t   descriptor;
    Resource*       resource;
};

struct MTCmdBeginRenderPass
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

struct MTCmdDraw
{
    NSUInteger vertexStart;
    NSUInteger vertexCount;
    NSUInteger instanceCount;
    NSUInteger baseInstance;
};

struct MTCmdDrawIndexed
{
    NSUInteger indexCount;
    NSUInteger firstIndex;
    NSUInteger instanceCount;
    NSUInteger baseVertex;
    NSUInteger baseInstance;
};

struct MTCmdDispatchThreads
{
    MTLSize threadgroups;
};

struct MTCmdDispatchThreadsIndirect
{
    id<MTLBuffer>   indirectBuffer;
    NSUInteger      indirectBufferOffset;
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
