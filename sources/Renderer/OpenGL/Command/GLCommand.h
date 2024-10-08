/*
 * GLCommand.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMMAND_H
#define LLGL_GL_COMMAND_H


#include <LLGL/CommandBufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Types.h>
#include "../RenderState/GLState.h"
#include "../Buffer/GLSharedContextVertexArray.h"
#include "../Profile/GLProfile.h"
#include <cstdint>


namespace LLGL
{


class RenderTarget;
class GLBuffer;
class GLBufferWithXFB;
class GLTexture;
class GLResourceHeap;
class GLPipelineState;
class GLQueryHeap;
class GLSwapChain;
class GLRenderTarget;
class GLRenderPass;
class GLDeferredCommandBuffer;
class GLEmulatedSampler;


struct GLCmdBufferSubData
{
    GLBuffer*       buffer;
    GLintptr        offset;
    GLsizeiptr      size;
//  std::int8_t     data[dataSize];
};

struct GLCmdCopyBufferSubData
{
    GLBuffer*   writeBuffer;
    GLBuffer*   readBuffer;
    GLintptr    readOffset;
    GLintptr    writeOffset;
    GLsizeiptr  size;
};

struct GLCmdClearBufferData
{
    GLBuffer*       buffer;
    std::uint32_t   data;
};

struct GLCmdClearBufferSubData
{
    GLBuffer*       buffer;
    GLintptr        offset;
    GLsizeiptr      size;
    std::uint32_t   data;
};

struct GLCmdCopyImageSubData
{
    GLTexture*  dstTexture;
    GLint       dstLevel;
    Offset3D    dstOffset;
    GLTexture*  srcTexture;
    GLint       srcLevel;
    Offset3D    srcOffset;
    Extent3D    extent;
};

// Used for both GLOpcodeCopyImageToBuffer and GLOpcodeCopyImageFromBuffer
struct GLCmdCopyImageBuffer
{
    GLTexture*      texture;
    TextureRegion   region;
    GLuint          bufferID;
    GLintptr        offset;
    GLsizei         size;
    GLint           rowLength;
    GLint           imageHeight;
};

struct GLCmdCopyFramebufferSubData
{
    GLTexture*  dstTexture;
    GLint       dstLevel;
    Offset3D    dstOffset;
    Offset2D    srcOffset;
    Extent2D    extent;
};

struct GLCmdGenerateMipmap
{
    GLTexture* texture;
};

struct GLCmdGenerateMipmapSubresource
{
    GLTexture*      texture;
    std::uint32_t   baseMipLevel;
    std::uint32_t   numMipLevels;
    std::uint32_t   baseArrayLayer;
    std::uint32_t   numArrayLayers;
};

struct GLCmdExecute
{
    const GLDeferredCommandBuffer* commandBuffer;
};

struct GLCmdViewport
{
    GLViewport      viewport;
    GLDepthRange    depthRange;
};

struct GLCmdViewportArray
{
    GLuint          first;
    GLsizei         count;
//  GLViewport      viewports[count];
//  GLDepthRange    depthRanges[count];
};

struct GLCmdScissor
{
    GLScissor scissor;
};

struct GLCmdScissorArray
{
    GLuint      first;
    GLsizei     count;
//  GLScissor   scissors[count];
};

struct GLCmdClearColor
{
    GLfloat color[4];
};

struct GLCmdClearDepth
{
    GLclamp_t depth;
};

struct GLCmdClearStencil
{
    GLint stencil;
};

struct GLCmdClear
{
    long flags;
};

struct GLCmdClearAttachmentsWithRenderPass
{
    const GLRenderPass* renderPass;
    std::uint32_t       numClearValues;
//  const ClearValue*   clearValues[numClearValues];
};

struct GLCmdClearBuffers
{
    std::uint32_t   numAttachments;
//  AttachmentClear attachments[numAttachments];
};

struct GLCmdResolveRenderTarget
{
    GLRenderTarget* renderTarget;
};

struct GLCmdBindVertexArray
{
    GLSharedContextVertexArray* vertexArray;
};

struct GLCmdBindElementArrayBufferToVAO
{
    GLuint id;
    bool   indexType16Bits;
};

struct GLCmdBindBufferBase
{
    GLBufferTarget  target;
    GLuint          index;
    GLuint          id;
};

struct GLCmdBindBuffersBase
{
    GLBufferTarget  target;
    GLuint          first;
    GLsizei         count;
//  GLuint          buffer[count];
};

struct GLCmdBeginBufferXfb
{
    GLBufferWithXFB*    bufferWithXfb;
    GLenum              primitiveMode;
};

struct GLCmdBeginTransformFeedback
{
    GLenum primitiveMove;
};

struct GLCmdBeginTransformFeedbackNV
{
    GLenum primitiveMove;
};

//struct GLCmdEndTransformFeedback {};

//struct GLCmdEndTransformFeedbackNV {};

struct GLCmdBindResourceHeap
{
    GLResourceHeap* resourceHeap;
    std::uint32_t   descriptorSet;
};

struct GLCmdBindRenderTarget
{
    RenderTarget* renderTarget;
};

struct GLCmdBindPipelineState
{
    GLPipelineState* pipelineState;
};

struct GLCmdSetBlendColor
{
    GLfloat color[4];
};

struct GLCmdSetStencilRef
{
    GLint   ref;
    GLenum  face;
};

struct GLCmdSetUniform
{
    GLuint      program;
    UniformType type;
    GLint       location;
    GLsizei     count;
    GLsizeiptr  size;
//  GLuint      buffer[size];
};

struct GLCmdBeginQuery
{
    GLQueryHeap*    queryHeap;
    std::uint32_t   query;
};

struct GLCmdEndQuery
{
    GLQueryHeap* queryHeap;
};

struct GLCmdBeginConditionalRender
{
    GLuint id;
    GLenum mode;
};

//struct GLCmdEndConditionalRender {};

struct GLCmdDrawArrays
{
    GLenum  mode;
    GLint   first;
    GLsizei count;
};

struct GLCmdDrawArraysInstanced
{
    GLenum  mode;
    GLint   first;
    GLsizei count;
    GLsizei instancecount;
};

struct GLCmdDrawArraysInstancedBaseInstance
{
    GLenum  mode;
    GLint   first;
    GLsizei count;
    GLsizei instancecount;
    GLuint  baseinstance;
};

struct GLCmdDrawArraysIndirect
{
    GLuint          id;
    std::uint32_t   numCommands;
    GLenum          mode;
    GLintptr        indirect;
    std::uint32_t   stride;
};

struct GLCmdDrawElements
{
    GLenum          mode;
    GLsizei         count;
    GLenum          type;
    const GLvoid*   indices;
};

struct GLCmdDrawElementsBaseVertex
{
    GLenum          mode;
    GLsizei         count;
    GLenum          type;
    const GLvoid*   indices;
    GLint           basevertex;
};

struct GLCmdDrawElementsInstanced
{
    GLenum          mode;
    GLsizei         count;
    GLenum          type;
    const GLvoid*   indices;
    GLsizei         instancecount;
};

struct GLCmdDrawElementsInstancedBaseVertex
{
    GLenum          mode;
    GLsizei         count;
    GLenum          type;
    const GLvoid*   indices;
    GLsizei         instancecount;
    GLint           basevertex;
};

struct GLCmdDrawElementsInstancedBaseVertexBaseInstance
{
    GLenum          mode;
    GLsizei         count;
    GLenum          type;
    const GLvoid*   indices;
    GLsizei         instancecount;
    GLint           basevertex;
    GLuint          baseinstance;
};

struct GLCmdDrawElementsIndirect
{
    GLuint          id;
    std::uint32_t   numCommands;
    GLenum          mode;
    GLenum          type;
    GLintptr        indirect;
    std::uint32_t   stride;
};

struct GLCmdMultiDrawArraysIndirect
{
    GLuint          id;
    GLenum          mode;
    const GLvoid*   indirect;
    GLsizei         drawcount;
    GLsizei         stride;
};

struct GLCmdMultiDrawElementsIndirect
{
    GLuint          id;
    GLenum          mode;
    GLenum          type;
    const GLvoid*   indirect;
    GLsizei         drawcount;
    GLsizei         stride;
};

struct GLCmdDrawTransformFeedback
{
    GLenum  mode;
    GLuint  xfbID;
};

struct GLCmdDrawEmulatedTransformFeedback
{
    GLenum              mode;
    GLBufferWithXFB*    bufferWithXfb;
};

struct GLCmdDispatchCompute
{
    GLuint numgroups[3];
};

struct GLCmdDispatchComputeIndirect
{
    GLuint      id;
    GLintptr    indirect;
};

struct GLCmdBindTexture
{
    GLuint      slot;
    GLTexture*  texture;
};

struct GLCmdBindImageTexture
{
    GLuint  unit;
    GLint   level;
    GLenum  format;
    GLuint  texture;
};

struct GLCmdBindSampler
{
    GLuint layer;
    GLuint sampler;
};

struct GLCmdBindEmulatedSampler
{
    GLuint                      layer;
    const GLEmulatedSampler*    sampler;
};

struct GLCmdMemoryBarrier
{
    GLbitfield barriers;
};

struct GLCmdPushDebugGroup
{
    GLenum      source;
    GLuint      id;
    GLsizei     length;
//  GLchar      name[length];
};

//struct GLCmdPopDebugGroup {};


} // /namespace LLGL


#endif



// ================================================================================
