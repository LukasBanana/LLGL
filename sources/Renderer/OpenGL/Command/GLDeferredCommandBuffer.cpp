/*
 * GLDeferredCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLDeferredCommandBuffer.h"
#include "GLCommand.h"
#include <LLGL/StaticLimits.h>

#include "../../TextureUtils.h"
#include "../GLSwapChain.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderPipeline.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"
#ifdef LLGL_GL_ENABLE_OPENGL2X
#   include "../Texture/GL2XSampler.h"
#endif

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLGraphicsPSO.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <algorithm>
#include <string.h>
#include <cstring> // std::strlen

#ifdef LLGL_ENABLE_JIT_COMPILER
#   include "GLCommandAssembler.h"
#endif // /LLGL_ENABLE_JIT_COMPILER


namespace LLGL
{


GLDeferredCommandBuffer::GLDeferredCommandBuffer(long flags, std::size_t initialBufferSize) :
    flags_  { flags             },
    buffer_ { initialBufferSize }
{
}

/* ----- Encoding ----- */

void GLDeferredCommandBuffer::Begin()
{
    /* Reset internal command buffer */
    buffer_.Clear();
    ResetRenderState();

    #ifdef LLGL_ENABLE_JIT_COMPILER

    /* Reset states relevant to the GL command assembler */
    executable_.reset();
    maxNumViewports_ = 0;
    maxNumScissors_  = 0;

    #endif // /LLGL_ENABLE_JIT_COMPILER
}

void GLDeferredCommandBuffer::End()
{
    #ifdef LLGL_ENABLE_JIT_COMPILER

    /* Generate native assembly only if command buffer will be submitted multiple times */
    if ((GetFlags() & CommandBufferFlags::MultiSubmit) != 0)
        executable_ = AssembleGLDeferredCommandBuffer(*this);

    #else

    /* Pack virtual command buffer if it has to be traversed multiple times */
    if ((GetFlags() & CommandBufferFlags::MultiSubmit) != 0)
        buffer_.Pack();

    #endif // /LLGL_ENABLE_JIT_COMPILER
}

void GLDeferredCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    if (IsPrimary())
    {
        /* Is this a secondary command buffer? */
        auto& cmdBufferGL = LLGL_CAST(const GLCommandBuffer&, deferredCommandBuffer);
        if (!cmdBufferGL.IsImmediateCmdBuffer())
        {
            auto& deferredCmdBufferGL = LLGL_CAST(const GLDeferredCommandBuffer&, cmdBufferGL);
            if (!deferredCmdBufferGL.IsPrimary())
            {
                /* Encode GL command */
                auto cmd = AllocCommand<GLCmdExecute>(GLOpcodeExecute);
                cmd->commandBuffer = &deferredCmdBufferGL;
            }
        }
    }
}

/* ----- Blitting ----- */

void GLDeferredCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto cmd = AllocCommand<GLCmdBufferSubData>(GLOpcodeBufferSubData, dataSize);
    {
        cmd->buffer = LLGL_CAST(GLBuffer*, &dstBuffer);
        cmd->offset = static_cast<GLintptr>(dstOffset);
        cmd->size   = static_cast<GLsizeiptr>(dataSize);
        ::memcpy(cmd + 1, data, dataSize);
    }
}

void GLDeferredCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto cmd = AllocCommand<GLCmdCopyBufferSubData>(GLOpcodeCopyBufferSubData);
    {
        cmd->writeBuffer    = LLGL_CAST(GLBuffer*, &dstBuffer);
        cmd->readBuffer     = LLGL_CAST(GLBuffer*, &srcBuffer);
        cmd->readOffset     = static_cast<GLintptr>(srcOffset);
        cmd->writeOffset    = static_cast<GLintptr>(dstOffset);
        cmd->size           = static_cast<GLsizeiptr>(size);
    }
}

void GLDeferredCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto cmd = AllocCommand<GLCmdCopyImageBuffer>(GLOpcodeCopyImageToBuffer);
    {
        cmd->texture        = LLGL_CAST(GLTexture*, &srcTexture);
        cmd->region         = srcRegion;
        cmd->bufferID       = LLGL_CAST(GLBuffer&, dstBuffer).GetID();
        cmd->offset         = static_cast<GLintptr>(dstOffset);
        cmd->size           = cmd->texture->GetMemoryFootprint(srcRegion.extent, srcRegion.subresource);
        cmd->rowLength      = static_cast<GLint>(rowStride);
        cmd->imageHeight    = static_cast<GLint>(rowStride > 0 ? layerStride / rowStride : 0);
    }
}

void GLDeferredCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    if (fillSize == Constants::wholeSize)
    {
        auto cmd = AllocCommand<GLCmdClearBufferData>(GLOpcodeClearBufferData);
        {
            cmd->buffer = LLGL_CAST(GLBuffer*, &dstBuffer);
            cmd->data   = value;
        }
    }
    else
    {
        auto cmd = AllocCommand<GLCmdClearBufferSubData>(GLOpcodeClearBufferSubData);
        {
            cmd->buffer = LLGL_CAST(GLBuffer*, &dstBuffer);
            cmd->offset = static_cast<GLintptr>(dstOffset);
            cmd->size   = static_cast<GLsizeiptr>(fillSize);
            cmd->data   = value;
        }
    }
}

void GLDeferredCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto cmd = AllocCommand<GLCmdCopyImageSubData>(GLOpcodeCopyImageSubData);
    {
        cmd->dstTexture = LLGL_CAST(GLTexture*, &dstTexture);
        cmd->dstLevel   = static_cast<GLint>(dstLocation.mipLevel);
        cmd->dstOffset  = CalcTextureOffset(dstTexture.GetType(), dstLocation.offset, dstLocation.arrayLayer);
        cmd->srcTexture = LLGL_CAST(GLTexture*, &srcTexture);
        cmd->srcLevel   = static_cast<GLint>(srcLocation.mipLevel);
        cmd->srcOffset  = CalcTextureOffset(srcTexture.GetType(), srcLocation.offset, srcLocation.arrayLayer);
        cmd->extent     = extent;
    }
}

void GLDeferredCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto cmd = AllocCommand<GLCmdCopyImageBuffer>(GLOpcodeCopyImageFromBuffer);
    {
        cmd->texture        = LLGL_CAST(GLTexture*, &dstTexture);
        cmd->region         = dstRegion;
        cmd->bufferID       = LLGL_CAST(GLBuffer&, srcBuffer).GetID();
        cmd->offset         = static_cast<GLintptr>(srcOffset);
        cmd->size           = cmd->texture->GetMemoryFootprint(dstRegion.extent, dstRegion.subresource);
        cmd->rowLength      = static_cast<GLint>(rowStride);
        cmd->imageHeight    = static_cast<GLint>(rowStride > 0 ? layerStride / rowStride : 0);
    }
}

void GLDeferredCommandBuffer::GenerateMips(Texture& texture)
{
    auto cmd = AllocCommand<GLCmdGenerateMipmap>(GLOpcodeGenerateMipmap);
    {
        cmd->texture = LLGL_CAST(GLTexture*, &texture);
    }
}

void GLDeferredCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto cmd = AllocCommand<GLCmdGenerateMipmapSubresource>(GLOpcodeGenerateMipmapSubresource);
    {
        cmd->texture        = LLGL_CAST(GLTexture*, &texture);
        cmd->baseMipLevel   = subresource.baseMipLevel;
        cmd->numMipLevels   = subresource.numMipLevels;
        cmd->baseArrayLayer = subresource.baseArrayLayer;
        cmd->numArrayLayers = subresource.numArrayLayers;
    }
}

/* ----- Viewport and Scissor ----- */

void GLDeferredCommandBuffer::SetViewport(const Viewport& viewport)
{
    #ifdef LLGL_ENABLE_JIT_COMPILER
    maxNumViewports_ = std::max(maxNumViewports_, 1u);
    #endif // /LLGL_ENABLE_JIT_COMPILER

    auto cmd = AllocCommand<GLCmdViewport>(GLOpcodeViewport);
    {
        cmd->viewport   = GLViewport{ viewport.x, viewport.y, viewport.width, viewport.height };
        cmd->depthRange = GLDepthRange{ viewport.minDepth, viewport.maxDepth };
    }
}

void GLDeferredCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    /* Clamp number of viewports to limit */
    numViewports = std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);

    #ifdef LLGL_ENABLE_JIT_COMPILER
    maxNumViewports_ = std::max(maxNumViewports_, numViewports);
    #endif // /LLGL_ENABLE_JIT_COMPILER

    /* Encode GL command */
    auto cmd = AllocCommand<GLCmdViewportArray>(GLOpcodeViewportArray, (sizeof(GLViewport) + sizeof(GLDepthRange))*numViewports);
    {
        cmd->first = 0;
        cmd->count = static_cast<GLsizei>(numViewports);

        auto viewportsGL = reinterpret_cast<GLViewport*>(cmd + 1);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            viewportsGL[i].x        = viewports[i].x;
            viewportsGL[i].y        = viewports[i].y;
            viewportsGL[i].width    = viewports[i].width;
            viewportsGL[i].height   = viewports[i].height;
        }

        auto depthRangesGL = reinterpret_cast<GLDepthRange*>(viewportsGL + numViewports);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            depthRangesGL[i].minDepth = static_cast<GLclamp_t>(viewports[i].minDepth);
            depthRangesGL[i].maxDepth = static_cast<GLclamp_t>(viewports[i].maxDepth);
        }
    }
}

void GLDeferredCommandBuffer::SetScissor(const Scissor& scissor)
{
    #ifdef LLGL_ENABLE_JIT_COMPILER
    maxNumScissors_ = std::max(maxNumScissors_, 1u);
    #endif // /LLGL_ENABLE_JIT_COMPILER

    auto cmd = AllocCommand<GLCmdScissor>(GLOpcodeScissor);
    cmd->scissor = GLScissor{ scissor.x, scissor.y, scissor.width, scissor.height };
}

void GLDeferredCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    /* Clamp number of scissors to limit */
    numScissors = std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);

    #ifdef LLGL_ENABLE_JIT_COMPILER
    maxNumScissors_ = std::max(maxNumScissors_, numScissors);
    #endif // /LLGL_ENABLE_JIT_COMPILER

    /* Encode GL command */
    auto cmd = AllocCommand<GLCmdScissorArray>(GLOpcodeScissorArray, sizeof(GLScissor)*numScissors);
    {
        cmd->first = 0;
        cmd->count = static_cast<GLsizei>(numScissors);

        auto scissorsGL = reinterpret_cast<GLScissor*>(cmd + 1);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            scissorsGL[i].x         = static_cast<GLint>(scissors[i].x);
            scissorsGL[i].y         = static_cast<GLint>(scissors[i].y);
            scissorsGL[i].width     = static_cast<GLsizei>(scissors[i].width);
            scissorsGL[i].height    = static_cast<GLsizei>(scissors[i].height);
        }
    }
}

/* ----- Input Assembly ------ */

void GLDeferredCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto& bufferWithVAO = LLGL_CAST(const GLBufferWithVAO&, buffer);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasNativeVAO())
        {
            auto cmd = AllocCommand<GLCmdBindGL2XVertexArray>(GLOpcodeBindGL2XVertexArray);
            cmd->vertexArrayGL2X = &(bufferWithVAO.GetVertexArrayGL2X());
        }
        else
        #endif // /LLGL_GL_ENABLE_OPENGL2X
        {
            auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpcodeBindVertexArray);
            cmd->vao = bufferWithVAO.GetVaoID();
        }
    }
}

void GLDeferredCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto& bufferArrayWithVAO = LLGL_CAST(const GLBufferArrayWithVAO&, bufferArray);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasNativeVAO())
        {
            auto cmd = AllocCommand<GLCmdBindGL2XVertexArray>(GLOpcodeBindGL2XVertexArray);
            cmd->vertexArrayGL2X = &(bufferArrayWithVAO.GetVertexArrayGL2X());
        }
        else
        #endif
        {
            auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpcodeBindVertexArray);
            cmd->vao = bufferArrayWithVAO.GetVaoID();
        }
    }
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    auto cmd = AllocCommand<GLCmdBindElementArrayBufferToVAO>(GLOpcodeBindElementArrayBufferToVAO);
    cmd->id = bufferGL.GetID();
    cmd->indexType16Bits = bufferGL.IsIndexType16Bits();
    SetIndexFormat(bufferGL.IsIndexType16Bits(), 0);
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    const bool indexType16Bits = (format == Format::R16UInt);
    auto cmd = AllocCommand<GLCmdBindElementArrayBufferToVAO>(GLOpcodeBindElementArrayBufferToVAO);
    cmd->id = bufferGL.GetID();
    cmd->indexType16Bits = indexType16Bits;
    SetIndexFormat(indexType16Bits, offset);
}

/* ----- Resource Heaps ----- */

void GLDeferredCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto cmd = AllocCommand<GLCmdBindResourceHeap>(GLOpcodeBindResourceHeap);
    cmd->resourceHeap   = LLGL_CAST(GLResourceHeap*, &resourceHeap);
    cmd->descriptorSet  = descriptorSet;
}

void GLDeferredCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    auto* pipelineLayoutGL = GetBoundPipelineLayout();
    if (pipelineLayoutGL == nullptr)
        return /*GL_INVALID_VALUE*/;

    const auto& bindingList = pipelineLayoutGL->GetBindings();
    if (!(descriptor < bindingList.size()))
        return /*GL_INVALID_INDEX*/;

    const auto& binding = bindingList[descriptor];
    switch (binding.type)
    {
        case GLResourceType_Invalid:
        break;

        case GLResourceType_UBO:
        {
            auto& bufferGL = LLGL_CAST(GLBuffer&, resource);
            BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, bufferGL, binding.slot);
        }
        break;

        case GLResourceType_SSBO:
        {
            auto& bufferGL = LLGL_CAST(GLBuffer&, resource);
            BindBufferBase(GLBufferTarget::SHADER_STORAGE_BUFFER, bufferGL, binding.slot);
        }
        break;

        case GLResourceType_Texture:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);
            BindTexture(textureGL, binding.slot);
        }
        break;

        case GLResourceType_Image:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);
            BindImageTexture(textureGL, binding.slot);
        }
        break;

        case GLResourceType_Sampler:
        {
            auto& samplerGL = LLGL_CAST(GLSampler&, resource);
            BindSampler(samplerGL, binding.slot);
        }
        break;

        case GLResourceType_GL2XSampler:
        {
            #ifdef LLGL_GL_ENABLE_OPENGL2X
            auto& samplerGL2X = LLGL_CAST(GL2XSampler&, resource);
            BindGL2XSampler(samplerGL2X, binding.slot);
            #endif // /LLGL_GL_ENABLE_OPENGL2X
        }
        break;
    }
}

void GLDeferredCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                /*stageFlags*/)
{
    GLCmdUnbindResources cmd;
    cmd.resetFlags = 0;

    cmd.first = static_cast<GLuint>(std::min(firstSlot, GLStateManager::g_maxNumResourceSlots - 1u));
    cmd.count = static_cast<GLsizei>(std::min(numSlots, GLStateManager::g_maxNumResourceSlots - cmd.first));

    if (cmd.count > 0)
    {
        switch (resourceType)
        {
            case ResourceType::Undefined:
            break;

            case ResourceType::Buffer:
            {
                if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                    cmd.resetUBO = 1;
                if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                    cmd.resetSSAO = 1;
                if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
                    cmd.resetTransformFeedback = 1;
            }
            break;

            case ResourceType::Texture:
            {
                if ((bindFlags & BindFlags::Sampled) != 0)
                    cmd.resetTextures = 1;
                if ((bindFlags & BindFlags::Storage) != 0)
                    cmd.resetImages = 1;
            }
            break;

            case ResourceType::Sampler:
            {
                cmd.resetSamplers = 1;
            }
            break;
        }

        if (cmd.resetFlags != 0)
            *AllocCommand<GLCmdUnbindResources>(GLOpcodeUnbindResources) = cmd;
    }
}

/* ----- Render Passes ----- */

void GLDeferredCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    auto cmd = AllocCommand<GLCmdBindRenderTarget>(GLOpcodeBindRenderTarget);
    {
        cmd->renderTarget = &renderTarget;
    }
    if (renderPass != nullptr)
    {
        auto cmd = AllocCommand<GLCmdClearAttachmentsWithRenderPass>(GLOpcodeClearAttachmentsWithRenderPass, sizeof(ClearValue)*numClearValues);
        {
            cmd->renderPass     = LLGL_CAST(const GLRenderPass*, renderPass);
            cmd->numClearValues = numClearValues;
            ::memcpy(cmd + 1, clearValues, sizeof(ClearValue)*numClearValues);
        }
    }
}

void GLDeferredCommandBuffer::EndRenderPass()
{
    // dummy
}

void GLDeferredCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if (flags != 0)
    {
        if ((flags & ClearFlags::Color) != 0)
        {
            auto cmd = AllocCommand<GLCmdClearColor>(GLOpcodeClearColor);
            cmd->color[0] = clearValue.color.r;
            cmd->color[1] = clearValue.color.g;
            cmd->color[2] = clearValue.color.b;
            cmd->color[3] = clearValue.color.a;
        }

        if ((flags & ClearFlags::Depth) != 0)
        {
            auto cmd = AllocCommand<GLCmdClearDepth>(GLOpcodeClearDepth);
            cmd->depth = static_cast<GLclamp_t>(clearValue.depth);
        }

        if ((flags & ClearFlags::Stencil) != 0)
        {
            auto cmd = AllocCommand<GLCmdClearStencil>(GLOpcodeClearStencil);
            cmd->stencil = static_cast<GLint>(clearValue.stencil);
        }

        auto cmd = AllocCommand<GLCmdClear>(GLOpcodeClear);
        cmd->flags = flags;
    }
}

void GLDeferredCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (numAttachments > 0)
    {
        auto cmd = AllocCommand<GLCmdClearBuffers>(GLOpcodeClearBuffers, sizeof(AttachmentClear)*numAttachments);
        {
            cmd->numAttachments = numAttachments;
            ::memcpy(cmd + 1, attachments, sizeof(AttachmentClear)*numAttachments);
        }
    }
}

/* ----- Pipeline States ----- */

void GLDeferredCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto cmd = AllocCommand<GLCmdBindPipelineState>(GLOpcodeBindPipelineState);
    cmd->pipelineState = LLGL_CAST(GLPipelineState*, &pipelineState);
    SetPipelineRenderState(*(cmd->pipelineState));
}

void GLDeferredCommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    auto cmd = AllocCommand<GLCmdSetBlendColor>(GLOpcodeSetBlendColor);
    {
        cmd->color[0] = color.r;
        cmd->color[1] = color.g;
        cmd->color[2] = color.b;
        cmd->color[3] = color.a;
    }
}

void GLDeferredCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    auto cmd = AllocCommand<GLCmdSetStencilRef>(GLOpcodeSetStencilRef);
    {
        cmd->ref    = static_cast<GLint>(reference);
        cmd->face   = GLTypes::Map(stencilFace);
    }
}

void GLDeferredCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    /* Data size must be a multiple of 4 bytes */
    if (dataSize == 0 || dataSize % 4 != 0 || data == nullptr)
        return /*GL_INVALID_VALUE*/;

    auto* boundPipelineState = GetBoundPipelineState();
    if (boundPipelineState == nullptr)
        return /*GL_INVALID_VALUE*/;

    auto* boundShaderPipeline = boundPipelineState->GetShaderPipeline();
    if (boundPipelineState == nullptr)
        return /*GL_INVALID_VALUE*/;

    const std::uint32_t dataSizeInWords = dataSize / 4;
    const auto& uniformMap = boundPipelineState->GetUniformMap();

    for (auto words = reinterpret_cast<const std::uint32_t*>(data), wordsEnd = words + dataSizeInWords; words != wordsEnd; ++first)
    {
        if (first >= uniformMap.size())
            return /*GL_INVALID_INDEX*/;

        /* Allocate GL command and copy data buffer */
        const auto& uniform = uniformMap[first];
        const std::uint32_t uniformSize = uniform.wordSize * 4;
        auto cmd = AllocCommand<GLCmdSetUniforms>(GLOpcodeSetUniforms, dataSize);
        {
            cmd->program    = boundShaderPipeline->GetID(); //TODO: must distinguish between GLShaderProgram and GLProgramPipeline
            cmd->type       = uniform.type;
            cmd->location   = uniform.location;
            cmd->count      = uniform.count;
            cmd->size       = static_cast<GLsizeiptr>(uniformSize);
            ::memcpy(cmd + 1, words, uniformSize);
        }
        words += uniform.wordSize;
    }
}

/* ----- Queries ----- */

void GLDeferredCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto cmd = AllocCommand<GLCmdBeginQuery>(GLOpcodeBeginQuery);
    {
        cmd->queryHeap  = LLGL_CAST(GLQueryHeap*, &queryHeap);
        cmd->query      = query;
    }
}

void GLDeferredCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto cmd = AllocCommand<GLCmdEndQuery>(GLOpcodeEndQuery);
    {
        cmd->queryHeap  = LLGL_CAST(GLQueryHeap*, &queryHeap);
        cmd->query      = query;
    }
}

void GLDeferredCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto cmd = AllocCommand<GLCmdBeginConditionalRender>(GLOpcodeBeginConditionalRender);
    {
        cmd->id     = LLGL_CAST(const GLQueryHeap&, queryHeap).GetID(query);
        cmd->mode   = GLTypes::Map(mode);
    }
}

void GLDeferredCommandBuffer::EndRenderCondition()
{
    AllocOpcode(GLOpcodeEndConditionalRender);
}

/* ----- Stream Output ------ */

#ifndef __APPLE__

#define LLGL_TRAP_TRANSFORM_FEEDBACK_NOT_SUPPORTED() \
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("stream-outputs (GL_EXT_transform_feedback, NV_transform_feedback)")

#endif

void GLDeferredCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    /* Bind transform feedback buffers */
    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);
    BindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, 0, numBuffers, buffers);

    /* Begin transform feedback section */
    #ifdef __APPLE__
    auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpcodeBeginTransformFeedback);
    cmd->primitiveMove = GetPrimitiveMode();
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpcodeBeginTransformFeedback);
        cmd->primitiveMove = GetPrimitiveMode();
    }
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedbackNV>(GLOpcodeBeginTransformFeedbackNV);
        cmd->primitiveMove = GetPrimitiveMode();
    }
    else
        LLGL_TRAP_TRANSFORM_FEEDBACK_NOT_SUPPORTED();
    #endif
}

void GLDeferredCommandBuffer::EndStreamOutput()
{
    #ifdef __APPLE__
    AllocOpcode(GLOpcodeEndTransformFeedback);
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        AllocOpcode(GLOpcodeEndTransformFeedback);
    else if (HasExtension(GLExt::NV_transform_feedback))
        AllocOpcode(GLOpcodeEndTransformFeedbackNV);
    else
        LLGL_TRAP_TRANSFORM_FEEDBACK_NOT_SUPPORTED();
    #endif
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type <GLintptr> to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

void GLDeferredCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    auto cmd = AllocCommand<GLCmdDrawArrays>(GLOpcodeDrawArrays);
    {
        cmd->mode   = GetDrawMode();
        cmd->first  = static_cast<GLint>(firstVertex);
        cmd->count  = static_cast<GLsizei>(numVertices);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    auto cmd = AllocCommand<GLCmdDrawElements>(GLOpcodeDrawElements);
    {
        cmd->mode       = GetDrawMode();
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = GetIndexType();
        cmd->indices    = GetIndicesOffset(firstIndex);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto cmd = AllocCommand<GLCmdDrawElementsBaseVertex>(GLOpcodeDrawElementsBaseVertex);
    {
        cmd->mode       = GetDrawMode();
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = GetIndexType();
        cmd->indices    = GetIndicesOffset(firstIndex);
        cmd->basevertex = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    auto cmd = AllocCommand<GLCmdDrawArraysInstanced>(GLOpcodeDrawArraysInstanced);
    {
        cmd->mode           = GetDrawMode();
        cmd->first          = static_cast<GLint>(firstVertex);
        cmd->count          = static_cast<GLsizei>(numVertices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
    }
}

void GLDeferredCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDrawArraysInstancedBaseInstance>(GLOpcodeDrawArraysInstancedBaseInstance);
    {
        cmd->mode           = GetDrawMode();
        cmd->first          = static_cast<GLint>(firstVertex);
        cmd->count          = static_cast<GLsizei>(numVertices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->baseinstance   = firstInstance;
    }
    #else
    ErrUnsupportedGLProc("glDrawArraysInstancedBaseInstance");
    #endif
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    auto cmd = AllocCommand<GLCmdDrawElementsInstanced>(GLOpcodeDrawElementsInstanced);
    {
        cmd->mode           = GetDrawMode();
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = GetIndexType();
        cmd->indices        = GetIndicesOffset(firstIndex);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertex>(GLOpcodeDrawElementsInstancedBaseVertex);
    {
        cmd->mode           = GetDrawMode();
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = GetIndexType();
        cmd->indices        = GetIndicesOffset(firstIndex);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->basevertex     = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertexBaseInstance>(GLOpcodeDrawElementsInstancedBaseVertexBaseInstance);
    {
        cmd->mode           = GetDrawMode();
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = GetIndexType();
        cmd->indices        = GetIndicesOffset(firstIndex);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->basevertex     = vertexOffset;
        cmd->baseinstance   = firstInstance;
    }
    #else
    ErrUnsupportedGLProc("glDrawElementsInstancedBaseVertexBaseInstance");
    #endif
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto cmd = AllocCommand<GLCmdDrawArraysIndirect>(GLOpcodeDrawArraysIndirect);
    {
        cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
        cmd->numCommands    = 1;
        cmd->mode           = GetDrawMode();
        cmd->indirect       = static_cast<GLintptr>(offset);
        cmd->stride         = 0;
    }
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        const GLintptr indirect = static_cast<GLintptr>(offset);
        auto cmd = AllocCommand<GLCmdMultiDrawArraysIndirect>(GLOpcodeMultiDrawArraysIndirect);
        {
            cmd->id         = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->mode       = GetDrawMode();
            cmd->indirect   = reinterpret_cast<const GLvoid*>(indirect);
            cmd->drawcount  = static_cast<GLsizei>(numCommands);
            cmd->stride     = static_cast<GLsizei>(stride);
        }
    }
    else
    #endif // /__APPLE__
    {
        auto cmd = AllocCommand<GLCmdDrawArraysIndirect>(GLOpcodeDrawArraysIndirect);
        {
            cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->numCommands    = numCommands;
            cmd->mode           = GetDrawMode();
            cmd->indirect       = static_cast<GLintptr>(offset);
            cmd->stride         = stride;
        }
    }
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto cmd = AllocCommand<GLCmdDrawElementsIndirect>(GLOpcodeDrawElementsIndirect);
    {
        cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
        cmd->numCommands    = 1;
        cmd->mode           = GetDrawMode();
        cmd->type           = GetIndexType();
        cmd->indirect       = static_cast<GLintptr>(offset);
        cmd->stride         = 0;
    }
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        const GLintptr indirect = static_cast<GLintptr>(offset);
        auto cmd = AllocCommand<GLCmdMultiDrawElementsIndirect>(GLOpcodeMultiDrawElementsIndirect);
        {
            cmd->id         = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->mode       = GetDrawMode();
            cmd->type       = GetIndexType();
            cmd->indirect   = reinterpret_cast<const GLvoid*>(indirect);
            cmd->drawcount  = static_cast<GLsizei>(numCommands);
            cmd->stride     = static_cast<GLsizei>(stride);
        }
    }
    else
    #endif // /__APPLE__
    {
        auto cmd = AllocCommand<GLCmdDrawElementsIndirect>(GLOpcodeDrawElementsIndirect);
        {
            cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->numCommands    = numCommands;
            cmd->mode           = GetDrawMode();
            cmd->type           = GetIndexType();
            cmd->indirect       = static_cast<GLintptr>(offset);
            cmd->stride         = stride;
        }
    }
}

/* ----- Compute ----- */

void GLDeferredCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDispatchCompute>(GLOpcodeDispatchCompute);
    {
        cmd->numgroups[0] = numWorkGroupsX;
        cmd->numgroups[1] = numWorkGroupsY;
        cmd->numgroups[2] = numWorkGroupsZ;
    }
    #else
    ErrUnsupportedGLProc("glDispatchCompute");
    #endif
}

void GLDeferredCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDispatchComputeIndirect>(GLOpcodeDispatchComputeIndirect);
    {
        cmd->id         = LLGL_CAST(const GLBuffer&, buffer).GetID();
        cmd->indirect   = static_cast<GLintptr>(offset);
    }
    #else
    ErrUnsupportedGLProc("glDispatchComputeIndirect");
    #endif
}

/* ----- Debugging ----- */

void GLDeferredCommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        /* Push debug group name into command stream with default ID no. */
        const GLint         maxLength       = GLStateManager::Get().GetLimits().maxDebugNameLength;
        const GLuint        id              = 0;
        const std::size_t   actualLength    = std::strlen(name);
        const std::size_t   croppedLength   = std::min(actualLength, static_cast<std::size_t>(maxLength));

        auto cmd = AllocCommand<GLCmdPushDebugGroup>(GLOpcodePushDebugGroup, croppedLength + 1);
        {
            cmd->source = GL_DEBUG_SOURCE_APPLICATION;
            cmd->id     = id;
            cmd->length = static_cast<GLsizei>(croppedLength);
            ::memcpy(cmd + 1, name, croppedLength + 1);
        }
    }
    #endif // /GL_KHR_debug
}

void GLDeferredCommandBuffer::PopDebugGroup()
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
        AllocOpcode(GLOpcodePopDebugGroup);
    #endif // /GL_KHR_debug
}

/* ----- Internal ----- */

bool GLDeferredCommandBuffer::IsImmediateCmdBuffer() const
{
    return false;
}

bool GLDeferredCommandBuffer::IsPrimary() const
{
    return ((GetFlags() & CommandBufferFlags::Secondary) == 0);
}


/*
 * ======= Private: =======
 */

void GLDeferredCommandBuffer::BindBufferBase(const GLBufferTarget bufferTarget, const GLBuffer& bufferGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindBufferBase>(GLOpcodeBindBufferBase);
    {
        cmd->target = bufferTarget;
        cmd->index  = slot;
        cmd->id     = bufferGL.GetID();
    }
}

void GLDeferredCommandBuffer::BindBuffersBase(const GLBufferTarget bufferTarget, std::uint32_t first, std::uint32_t count, const Buffer *const *const buffers)
{
    if (count > 1)
    {
        /* Encode as multi binding with <BindBuffersBase> */
        auto cmd = AllocCommand<GLCmdBindBuffersBase>(GLOpcodeBindBuffersBase, sizeof(GLuint)*count);
        {
            cmd->target = bufferTarget;
            cmd->first  = first;
            cmd->count  = static_cast<GLsizei>(count);
            auto bufferIDs = reinterpret_cast<GLuint*>(cmd + 1);
            for (std::uint32_t i = 0; i < count; ++i)
            {
                auto bufferGL = LLGL_CAST(const GLBuffer*, buffers[i]);
                bufferIDs[i] = bufferGL->GetID();
            }
        }
    }
    else if (count == 1)
    {
        /* Encode as single binding with <BindBufferBase> */
        auto bufferGL = LLGL_CAST(const GLBuffer*, buffers[0]);
        BindBufferBase(bufferTarget, *bufferGL, first);
    }
}

void GLDeferredCommandBuffer::BindTexture(GLTexture& textureGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindTexture>(GLOpcodeBindTexture);
    {
        cmd->slot       = slot;
        cmd->texture    = &textureGL;
    }
}

void GLDeferredCommandBuffer::BindImageTexture(const GLTexture& textureGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindImageTexture>(GLOpcodeBindImageTexture);
    {
        cmd->unit       = slot;
        cmd->level      = 0;
        cmd->format     = textureGL.GetGLInternalFormat();
        cmd->texture    = textureGL.GetID();
    }
}

void GLDeferredCommandBuffer::BindSampler(const GLSampler& samplerGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindSampler>(GLOpcodeBindSampler);
    {
        cmd->layer      = slot;
        cmd->sampler    = samplerGL.GetID();
    }
}

#ifdef LLGL_GL_ENABLE_OPENGL2X
void GLDeferredCommandBuffer::BindGL2XSampler(const GL2XSampler& samplerGL2X, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindGL2XSampler>(GLOpcodeBindGL2XSampler);
    {
        cmd->layer          = slot;
        cmd->samplerGL2X    = &samplerGL2X;
    }
}
#endif

void GLDeferredCommandBuffer::AllocOpcode(const GLOpcode opcode)
{
    buffer_.AllocOpcode(opcode);
}

template <typename TCommand>
TCommand* GLDeferredCommandBuffer::AllocCommand(const GLOpcode opcode, std::size_t payloadSize)
{
    return buffer_.AllocCommand<TCommand>(opcode, payloadSize);
}


} // /namespace LLGL



// ================================================================================
