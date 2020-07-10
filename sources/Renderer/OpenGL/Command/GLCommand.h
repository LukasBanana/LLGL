/*
 * GLCommand.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_H
#define LLGL_GL_COMMAND_H


#include <LLGL/CommandBufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Types.h>
#include "../RenderState/GLState.h"
#include "../GLProfile.h"
#include <cstdint>


namespace LLGL
{


class RenderTarget;
class GLBuffer;
class GLTexture;
class GLResourceHeap;
class GLPipelineState;
class GLQueryHeap;
class GLRenderContext;
class GLRenderTarget;
class GLRenderPass;
class GLDeferredCommandBuffer;
class GL2XVertexArray;


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

struct GLCmdSetAPIDepState
{
    OpenGLDependentStateDescriptor desc;
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

struct GLCmdClearBuffers
{
    std::uint32_t   numAttachments;
//  AttachmentClear attachments[numAttachments];
};

struct GLCmdBindVertexArray
{
    GLuint vao;
};

struct GLCmdBindGL2XVertexArray
{
    const GL2XVertexArray* vertexArrayGL2X;
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
    std::uint32_t   firstSet;
};

struct GLCmdBindRenderPass
{
    RenderTarget*       renderTarget;
    const GLRenderPass* renderPass;
    std::uint32_t       numClearValues;
    GLClearValue        defaultClearValue;
//  const ClearValue*   clearValues[numClearValues];
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

struct GLCmdSetUniforms
{
    GLuint      program;
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
    GLQueryHeap*    queryHeap;
    std::uint32_t   query;
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
    std::uint32_t       slot;
    const GLTexture*    texture;
};

struct GLCmdBindSampler
{
    std::uint32_t   slot;
    GLuint          sampler;
};

struct GLCmdUnbindResources
{
    GLuint              first;
    GLsizei             count;
    union
    {
        std::uint8_t    resetFlags;
        struct
        {
            std::uint8_t    resetUBO                : 1;
            std::uint8_t    resetSSAO               : 1;
            std::uint8_t    resetTransformFeedback  : 1;
            std::uint8_t    resetTextures           : 1;
            std::uint8_t    resetImages             : 1;
            std::uint8_t    resetSamplers           : 1;
        };
    };
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
