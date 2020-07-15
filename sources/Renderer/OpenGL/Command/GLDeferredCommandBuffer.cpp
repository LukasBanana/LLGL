/*
 * GLDeferredCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLDeferredCommandBuffer.h"
#include "GLCommand.h"
#include <LLGL/StaticLimits.h>

#include "../../TextureUtils.h"
#include "../GLRenderContext.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"

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


GLDeferredCommandBuffer::GLDeferredCommandBuffer(long flags, std::size_t reservedSize) :
    flags_ { flags }
{
    buffer_.reserve(reservedSize);
}

/* ----- Encoding ----- */

void GLDeferredCommandBuffer::Begin()
{
    /* Reset internal command buffer */
    buffer_.clear();
    boundShaderProgram_ = 0;

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
    auto cmd = AllocCommand<GLCmdViewportArray>(GLOpcodeViewportArray, sizeof(GLViewport)*numViewports);
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

/* ----- Clear ----- */

void GLDeferredCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearColor>(GLOpcodeClearColor);
    {
        cmd->color[0] = color.r;
        cmd->color[1] = color.g;
        cmd->color[2] = color.b;
        cmd->color[3] = color.a;
    }

    /* Store as default clear value */
    clearValue_.color[0] = color.r;
    clearValue_.color[1] = color.g;
    clearValue_.color[2] = color.b;
    clearValue_.color[3] = color.a;
}

void GLDeferredCommandBuffer::SetClearDepth(float depth)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearDepth>(GLOpcodeClearDepth);
    cmd->depth = static_cast<GLclamp_t>(depth);

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLDeferredCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearStencil>(GLOpcodeClearStencil);
    cmd->stencil = static_cast<GLint>(stencil);

    /* Store as default clear value */
    clearValue_.stencil = cmd->stencil;
}

void GLDeferredCommandBuffer::Clear(long flags)
{
    auto cmd = AllocCommand<GLCmdClear>(GLOpcodeClear);
    cmd->flags = flags;
}

void GLDeferredCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    auto cmd = AllocCommand<GLCmdClearBuffers>(GLOpcodeClearBuffers, sizeof(AttachmentClear)*numAttachments);
    {
        cmd->numAttachments = numAttachments;
        ::memcpy(cmd + 1, attachments, sizeof(AttachmentClear)*numAttachments);
    }
}

/* ----- Input Assembly ------ */

void GLDeferredCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto& bufferWithVAO = LLGL_CAST(const GLBufferWithVAO&, buffer);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasExtension(GLExt::ARB_vertex_array_object))
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
        if (!HasExtension(GLExt::ARB_vertex_array_object))
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
    SetIndexFormat(renderState_, bufferGL.IsIndexType16Bits(), 0);
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    const bool indexType16Bits = (format == Format::R16UInt);
    auto cmd = AllocCommand<GLCmdBindElementArrayBufferToVAO>(GLOpcodeBindElementArrayBufferToVAO);
    cmd->id = bufferGL.GetID();
    cmd->indexType16Bits = indexType16Bits;
    SetIndexFormat(renderState_, indexType16Bits, offset);
}

/* ----- Resource Heaps ----- */

void GLDeferredCommandBuffer::SetResourceHeap(
    ResourceHeap&           resourceHeap,
    std::uint32_t           firstSet,
    const PipelineBindPoint /*bindPoint*/)
{
    auto cmd = AllocCommand<GLCmdBindResourceHeap>(GLOpcodeBindResourceHeap);
    cmd->resourceHeap   = LLGL_CAST(GLResourceHeap*, &resourceHeap);
    cmd->firstSet       = firstSet;
}

void GLDeferredCommandBuffer::SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long stageFlags)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            auto& bufferGL = LLGL_CAST(GLBuffer&, resource);

            /* Bind uniform buffer (UBO) or shader storage buffer (SSBO) */
            if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, bufferGL, slot);
            if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                BindBufferBase(GLBufferTarget::SHADER_STORAGE_BUFFER, bufferGL, slot);
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);

            /* Bind sampled texture resource */
            if ((bindFlags & BindFlags::Sampled) != 0)
                BindTexture(textureGL, slot);

            //TODO: support image storage types (e.g. <image2D> in GLSL)
            /*if ((bindFlags & BindFlags::Storage) != 0)
            {
            }*/
        }
        break;

        case ResourceType::Sampler:
        {
            auto& samplerGL = LLGL_CAST(GLSampler&, resource);
            BindSampler(samplerGL, slot);
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
    auto cmd = AllocCommand<GLCmdBindRenderPass>(GLOpcodeBindRenderPass, sizeof(ClearValue)*numClearValues);
    {
        cmd->renderTarget       = &renderTarget;
        cmd->renderPass         = (renderPass != nullptr ? LLGL_CAST(const GLRenderPass*, renderPass) : nullptr);
        cmd->numClearValues     = numClearValues;
        cmd->defaultClearValue  = clearValue_;
        ::memcpy(cmd + 1, clearValues, sizeof(ClearValue)*numClearValues);
    }
}

void GLDeferredCommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void GLDeferredCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto cmd = AllocCommand<GLCmdBindPipelineState>(GLOpcodeBindPipelineState);
    cmd->pipelineState = LLGL_CAST(GLPipelineState*, &pipelineState);

    /* Store draw mode, primitive mode, and shader program */
    boundShaderProgram_ = cmd->pipelineState->GetShaderProgram()->GetID();

    if (cmd->pipelineState->IsGraphicsPSO())
    {
        auto graphicsPSO = LLGL_CAST(GLGraphicsPSO*, cmd->pipelineState);
        renderState_.drawMode       = graphicsPSO->GetDrawMode();
        renderState_.primitiveMode  = graphicsPSO->GetPrimitiveMode();
    }
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

void GLDeferredCommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    GLDeferredCommandBuffer::SetUniforms(location, 1, data, dataSize);
}

void GLDeferredCommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    /* Data size must be a multiple of 4 bytes */
    if (dataSize == 0 || dataSize % 4 != 0)
        return;

    /* Allocate GL command and copy data buffer */
    auto cmd = AllocCommand<GLCmdSetUniforms>(GLOpcodeSetUniforms, dataSize);
    {
        cmd->program    = boundShaderProgram_;
        cmd->location   = static_cast<GLint>(location);
        cmd->count      = static_cast<GLsizei>(count);
        cmd->size       = static_cast<GLsizeiptr>(dataSize);
        ::memcpy(cmd + 1, data, dataSize);
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
    AllocOpCode(GLOpcodeEndConditionalRender);
}

/* ----- Stream Output ------ */

#ifndef __APPLE__

[[noreturn]]
static void ErrTransformFeedbackNotSupported(const char* funcName)
{
    ThrowNotSupportedExcept(funcName, "stream-outputs (GL_EXT_transform_feedback, NV_transform_feedback)");
}

#endif

void GLDeferredCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    /* Bind transform feedback buffers */
    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);
    BindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, 0, numBuffers, buffers);

    /* Begin transform feedback section */
    #ifdef __APPLE__
    auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpcodeBeginTransformFeedback);
    cmd->primitiveMove = renderState_.primitiveMode;
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpcodeBeginTransformFeedback);
        cmd->primitiveMove = renderState_.primitiveMode;
    }
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedbackNV>(GLOpcodeBeginTransformFeedbackNV);
        cmd->primitiveMove = renderState_.primitiveMode;
    }
    else
        ErrTransformFeedbackNotSupported(__FUNCTION__);
    #endif
}

void GLDeferredCommandBuffer::EndStreamOutput()
{
    #ifdef __APPLE__
    AllocOpCode(GLOpcodeEndTransformFeedback);
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        AllocOpCode(GLOpcodeEndTransformFeedback);
    else if (HasExtension(GLExt::NV_transform_feedback))
        AllocOpCode(GLOpcodeEndTransformFeedbackNV);
    else
        ErrTransformFeedbackNotSupported(__FUNCTION__);
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
        cmd->mode   = renderState_.drawMode;
        cmd->first  = static_cast<GLint>(firstVertex);
        cmd->count  = static_cast<GLsizei>(numVertices);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElements>(GLOpcodeDrawElements);
    {
        cmd->mode       = renderState_.drawMode;
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = renderState_.indexBufferDataType;
        cmd->indices    = reinterpret_cast<const GLvoid*>(indices);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsBaseVertex>(GLOpcodeDrawElementsBaseVertex);
    {
        cmd->mode       = renderState_.drawMode;
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = renderState_.indexBufferDataType;
        cmd->indices    = reinterpret_cast<const GLvoid*>(indices);
        cmd->basevertex = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    auto cmd = AllocCommand<GLCmdDrawArraysInstanced>(GLOpcodeDrawArraysInstanced);
    {
        cmd->mode           = renderState_.drawMode;
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
        cmd->mode           = renderState_.drawMode;
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
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstanced>(GLOpcodeDrawElementsInstanced);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertex>(GLOpcodeDrawElementsInstancedBaseVertex);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->basevertex     = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertexBaseInstance>(GLOpcodeDrawElementsInstancedBaseVertexBaseInstance);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
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
        cmd->mode           = renderState_.drawMode;
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
            cmd->mode       = renderState_.drawMode;
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
            cmd->mode           = renderState_.drawMode;
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
        cmd->mode           = renderState_.drawMode;
        cmd->type           = renderState_.indexBufferDataType;
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
            cmd->mode       = renderState_.drawMode;
            cmd->type       = renderState_.indexBufferDataType;
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
            cmd->mode           = renderState_.drawMode;
            cmd->type           = renderState_.indexBufferDataType;
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
        AllocOpCode(GLOpcodePopDebugGroup);
    #endif // /GL_KHR_debug
}

/* ----- Extensions ----- */

void GLDeferredCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    if (stateDesc != nullptr && stateDescSize == sizeof(OpenGLDependentStateDescriptor))
    {
        auto cmd = AllocCommand<GLCmdSetAPIDepState>(GLOpcodeSetAPIDepState);
        cmd->desc = *reinterpret_cast<const OpenGLDependentStateDescriptor*>(stateDesc);
    }
}

/* ----- Internal ----- */

bool GLDeferredCommandBuffer::IsImmediateCmdBuffer() const
{
    return false;
}

bool GLDeferredCommandBuffer::IsPrimary() const
{
    return ((GetFlags() & CommandBufferFlags::DeferredSubmit) == 0);
}


/*
 * ======= Private: =======
 */

void GLDeferredCommandBuffer::BindBufferBase(const GLBufferTarget bufferTarget, GLBuffer& bufferGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindBufferBase>(GLOpcodeBindBufferBase);
    {
        cmd->target = bufferTarget;
        cmd->index  = slot;
        cmd->id     = bufferGL.GetID();
    }
}

void GLDeferredCommandBuffer::BindBuffersBase(const GLBufferTarget bufferTarget, std::uint32_t first, std::uint32_t count, Buffer* const * buffers)
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
                auto bufferGL = LLGL_CAST(GLBuffer*, buffers[i]);
                bufferIDs[i] = bufferGL->GetID();
            }
        }
    }
    else if (count == 1)
    {
        /* Encode as single binding with <BindBufferBase> */
        auto bufferGL = LLGL_CAST(GLBuffer*, buffers[0]);
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

void GLDeferredCommandBuffer::BindSampler(GLSampler& samplerGL, std::uint32_t slot)
{
    auto cmd = AllocCommand<GLCmdBindSampler>(GLOpcodeBindSampler);
    {
        cmd->slot       = slot;
        cmd->sampler    = samplerGL.GetID();
    }
}

void GLDeferredCommandBuffer::AllocOpCode(const GLOpcode opcode)
{
    buffer_.push_back(opcode);
}

template <typename T>
T* GLDeferredCommandBuffer::AllocCommand(const GLOpcode opcode, std::size_t extraSize)
{
    /* Resize internal buffer for opcode, command structure, and extra size */
    auto offset = buffer_.size();
    {
        buffer_.resize(offset + sizeof(opcode) + sizeof(T) + extraSize);
        buffer_[offset] = opcode;
    }
    return reinterpret_cast<T*>(&(buffer_[offset + sizeof(opcode)]));
}


} // /namespace LLGL



// ================================================================================
