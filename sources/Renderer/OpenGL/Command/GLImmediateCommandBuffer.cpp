/*
 * GLImmediateCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLImmediateCommandBuffer.h"
#include "GLDeferredCommandBuffer.h"
#include "GLCommandExecutor.h"
#include <LLGL/StaticLimits.h>

#include "../../TextureUtils.h"
#include "../GLRenderContext.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../GLProfile.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"
#include "../Texture/GLMipGenerator.h"

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLGraphicsPSO.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <cstring> // std::strlen


namespace LLGL
{


GLImmediateCommandBuffer::GLImmediateCommandBuffer(const std::shared_ptr<GLStateManager>& stateMngr) :
    stateMngr_ { stateMngr }
{
}

/* ----- Encoding ----- */

void GLImmediateCommandBuffer::Begin()
{
    // dummy
}

void GLImmediateCommandBuffer::End()
{
    // dummy
}

void GLImmediateCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferGL = LLGL_CAST(const GLCommandBuffer&, deferredCommandBuffer);
    ExecuteGLCommandBuffer(cmdBufferGL, *stateMngr_);
}

/* ----- Blitting ----- */

void GLImmediateCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    dstBufferGL.BufferSubData(static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(dataSize), data);
}

void GLImmediateCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    auto& srcBufferGL = LLGL_CAST(GLBuffer&, srcBuffer);
    dstBufferGL.CopyBufferSubData(
        srcBufferGL,
        static_cast<GLintptr>(srcOffset),
        static_cast<GLintptr>(dstOffset),
        static_cast<GLsizeiptr>(size)
    );
}

void GLImmediateCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    auto& srcTextureGL = LLGL_CAST(GLTexture&, srcTexture);
    srcTextureGL.CopyImageToBuffer(
        srcRegion,
        dstBufferGL.GetID(),
        static_cast<GLintptr>(dstOffset),
        srcTextureGL.GetMemoryFootprint(srcRegion.extent, srcRegion.subresource),
        static_cast<GLint>(rowStride),
        static_cast<GLint>(rowStride > 0 ? layerStride / rowStride : 0)
    );
}

void GLImmediateCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    if (fillSize == Constants::wholeSize)
        dstBufferGL.ClearBufferData(value);
    else
        dstBufferGL.ClearBufferSubData(static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(fillSize), value);
}

void GLImmediateCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureGL = LLGL_CAST(GLTexture&, dstTexture);
    auto& srcTextureGL = LLGL_CAST(GLTexture&, srcTexture);
    dstTextureGL.CopyImageSubData(
        static_cast<GLint>(dstLocation.mipLevel),
        CalcTextureOffset(dstTexture.GetType(), dstLocation.offset, dstLocation.arrayLayer),
        srcTextureGL,
        static_cast<GLint>(srcLocation.mipLevel),
        CalcTextureOffset(srcTexture.GetType(), srcLocation.offset, srcLocation.arrayLayer),
        extent
    );
}

void GLImmediateCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureGL = LLGL_CAST(GLTexture&, dstTexture);
    auto& srcBufferGL = LLGL_CAST(GLBuffer&, srcBuffer);
    dstTextureGL.CopyImageFromBuffer(
        dstRegion,
        srcBufferGL.GetID(),
        static_cast<GLintptr>(srcOffset),
        dstTextureGL.GetMemoryFootprint(dstRegion.extent, dstRegion.subresource),
        static_cast<GLint>(rowStride),
        static_cast<GLint>(rowStride > 0 ? layerStride / rowStride : 0)
    );
}

void GLImmediateCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLMipGenerator::Get().GenerateMipsForTexture(*stateMngr_, textureGL);
}

void GLImmediateCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLMipGenerator::Get().GenerateMipsRangeForTexture(
        *stateMngr_,
        textureGL,
        subresource.baseMipLevel,
        subresource.numMipLevels,
        subresource.baseArrayLayer,
        subresource.numArrayLayers
    );
}

/* ----- Viewport and Scissor ----- */

void GLImmediateCommandBuffer::SetViewport(const Viewport& viewport)
{
    /* Setup GL viewport and depth-range */
    GLViewport viewportGL{ viewport.x, viewport.y, viewport.width, viewport.height };
    GLDepthRange depthRangeGL{ viewport.minDepth, viewport.maxDepth };

    /* Set final state */
    stateMngr_->SetViewport(viewportGL);
    stateMngr_->SetDepthRange(depthRangeGL);
}

void GLImmediateCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    GLViewport viewportsGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    GLDepthRange depthRangesGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Setup GL viewports and depth-ranges */
    auto count = static_cast<GLsizei>(std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

    for (GLsizei i = 0; i < count; ++i)
    {
        /* Copy GL viewport data */
        viewportsGL[i].x        = viewports[i].x;
        viewportsGL[i].y        = viewports[i].y;
        viewportsGL[i].width    = viewports[i].width;
        viewportsGL[i].height   = viewports[i].height;

        /* Copy GL depth-range data */
        depthRangesGL[i].minDepth = static_cast<GLclamp_t>(viewports[i].minDepth);
        depthRangesGL[i].maxDepth = static_cast<GLclamp_t>(viewports[i].maxDepth);
    }

    /* Submit viewports and depth-ranges to state manager */
    stateMngr_->SetViewportArray(0, count, viewportsGL);
    stateMngr_->SetDepthRangeArray(0, count, depthRangesGL);
}

void GLImmediateCommandBuffer::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL{ scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLImmediateCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    GLScissor scissorsGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Setup GL scissors */
    auto count = static_cast<GLsizei>(std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

    for (GLsizei i = 0; i < count; ++i)
    {
        /* Copy GL scissor data */
        scissorsGL[i].x         = static_cast<GLint>(scissors[i].x);
        scissorsGL[i].y         = static_cast<GLint>(scissors[i].y);
        scissorsGL[i].width     = static_cast<GLsizei>(scissors[i].width);
        scissorsGL[i].height    = static_cast<GLsizei>(scissors[i].height);
    }

    /* Submit scissors to state manager */
    stateMngr_->SetScissorArray(0, count, scissorsGL);
}

/* ----- Clear ----- */

void GLImmediateCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    /* Submit clear value to GL */
    glClearColor(color.r, color.g, color.b, color.a);

    /* Store as default clear value */
    clearValue_.color[0] = color.r;
    clearValue_.color[1] = color.g;
    clearValue_.color[2] = color.b;
    clearValue_.color[3] = color.a;
}

void GLImmediateCommandBuffer::SetClearDepth(float depth)
{
    /* Submit clear value to GL */
    GLProfile::ClearDepth(static_cast<GLclamp_t>(depth));

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLImmediateCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Submit clear value to GL */
    glClearStencil(static_cast<GLint>(stencil));

    /* Store as default clear value */
    clearValue_.stencil = static_cast<GLint>(stencil);
}

void GLImmediateCommandBuffer::Clear(long flags)
{
    stateMngr_->Clear(flags);
}

void GLImmediateCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    stateMngr_->ClearBuffers(numAttachments, attachments);
}

/* ----- Input Assembly ------ */

void GLImmediateCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferGL = LLGL_CAST(GLBufferWithVAO&, buffer);

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasExtension(GLExt::ARB_vertex_array_object))
        {
            /* Bind vertex array with emulator (for GL 2.x compatibility) */
            vertexBufferGL.GetVertexArrayGL2X().Bind(*stateMngr_);
        }
        else
        #endif // /LLGL_GL_ENABLE_OPENGL2X
        {
            /* Bind vertex array with native VAO */
            stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
        }
    }
}

void GLImmediateCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferArrayGL = LLGL_CAST(GLBufferArrayWithVAO&, bufferArray);

        #ifdef LLGL_GL_ENABLE_OPENGL2X
        if (!HasExtension(GLExt::ARB_vertex_array_object))
        {
            /* Bind vertex array with emulator (for GL 2.x compatibility) */
            vertexBufferArrayGL.GetVertexArrayGL2X().Bind(*stateMngr_);
        }
        else
        #endif // /LLGL_GL_ENABLE_OPENGL2X
        {
            /* Bind vertex array with native VAO */
            stateMngr_->BindVertexArray(vertexBufferArrayGL.GetVaoID());
        }
    }
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID(), bufferGL.IsIndexType16Bits());
    SetIndexFormat(renderState_, bufferGL.IsIndexType16Bits(), 0);
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    const bool indexType16Bits = (format == Format::R16UInt);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID(), indexType16Bits);
    SetIndexFormat(renderState_, indexType16Bits, offset);
}

/* ----- Resource Heaps ----- */

void GLImmediateCommandBuffer::SetResourceHeap(
    ResourceHeap&           resourceHeap,
    std::uint32_t           firstSet,
    const PipelineBindPoint /*bindPoint*/)
{
    auto& resourceHeapGL = LLGL_CAST(GLResourceHeap&, resourceHeap);
    resourceHeapGL.Bind(*stateMngr_, firstSet);
}

void GLImmediateCommandBuffer::SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long /*stageFlags*/)
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
                stateMngr_->BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, slot, bufferGL.GetID());
            if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                stateMngr_->BindBufferBase(GLBufferTarget::SHADER_STORAGE_BUFFER, slot, bufferGL.GetID());
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);

            /* Bind sampled texture resource */
            if ((bindFlags & BindFlags::Sampled) != 0)
            {
                stateMngr_->ActiveTexture(slot);
                stateMngr_->BindGLTexture(textureGL);
            }

            //TODO: support image storage types (e.g. <image2D> in GLSL)
            /*if ((bindFlags & BindFlags::Storage) != 0)
            {
            }*/
        }
        break;

        case ResourceType::Sampler:
        {
            auto& samplerGL = LLGL_CAST(GLSampler&, resource);
            stateMngr_->BindSampler(slot, samplerGL.GetID());
        }
        break;
    }
}

void GLImmediateCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                /*stageFlags*/)
{
    if (numSlots > 0)
    {
        auto first = static_cast<GLuint>(std::min(firstSlot, GLStateManager::g_maxNumResourceSlots - 1u));
        auto count = static_cast<GLsizei>(std::min(numSlots, GLStateManager::g_maxNumResourceSlots - first));

        switch (resourceType)
        {
            case ResourceType::Undefined:
            break;

            case ResourceType::Buffer:
            {
                if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::UNIFORM_BUFFER, first, count);
                if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::SHADER_STORAGE_BUFFER, first, count);
                if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, first, count);
            }
            break;

            case ResourceType::Texture:
            {
                if ((bindFlags & BindFlags::Sampled) != 0)
                    stateMngr_->UnbindTextures(first, count);
                if ((bindFlags & BindFlags::Storage) != 0)
                    stateMngr_->UnbindImageTextures(first, count);
            }
            break;

            case ResourceType::Sampler:
            {
                stateMngr_->UnbindSamplers(first, count);
            }
            break;
        }
    }
}

/* ----- Render Passes ----- */

void GLImmediateCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    stateMngr_->BindRenderPass(renderTarget, renderPass, numClearValues, clearValues, clearValue_);
}

void GLImmediateCommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void GLImmediateCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Bind graphics pipeline render states */
    auto& pipelineStateGL = LLGL_CAST(GLPipelineState&, pipelineState);
    pipelineStateGL.Bind(*stateMngr_);

    /* Store draw and primitive mode */
    if (pipelineStateGL.IsGraphicsPSO())
    {
        auto& graphicsPSO = LLGL_CAST(GLGraphicsPSO&, pipelineStateGL);
        renderState_.drawMode       = graphicsPSO.GetDrawMode();
        renderState_.primitiveMode  = graphicsPSO.GetPrimitiveMode();
    }
}

void GLImmediateCommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    stateMngr_->SetBlendColor(color.Ptr());
}

void GLImmediateCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    stateMngr_->SetStencilRef(static_cast<GLint>(reference), GLTypes::Map(stencilFace));
}

void GLImmediateCommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    GLImmediateCommandBuffer::SetUniforms(location, 1, data, dataSize);
}

void GLImmediateCommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    /* Data size must be a multiple of 4 bytes */
    if (dataSize == 0 || dataSize % 4 != 0)
        return;

    GLSetUniformsByLocation(
        stateMngr_->GetBoundShaderProgram(),
        static_cast<GLint>(location),
        static_cast<GLsizei>(count),
        data
    );
}

/* ----- Queries ----- */

void GLImmediateCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.Begin(query);
}

void GLImmediateCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.End(query);
}

void GLImmediateCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    #ifdef LLGL_GLEXT_CONDITIONAL_RENDER
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    glBeginConditionalRender(queryHeapGL.GetID(query), GLTypes::Map(mode));
    #endif
}

void GLImmediateCommandBuffer::EndRenderCondition()
{
    #ifdef LLGL_GLEXT_CONDITIONAL_RENDER
    glEndConditionalRender();
    #endif
}

/* ----- Stream Output ------ */

void GLImmediateCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    /* Bind transform feedback buffers */
    GLuint soTargets[LLGL_MAX_NUM_SO_BUFFERS];
    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        auto bufferGL = LLGL_CAST(GLBuffer*, buffers[i]);
        soTargets[i] = bufferGL->GetID();
    }

    stateMngr_->BindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, 0, static_cast<GLsizei>(numBuffers), soTargets);

    /* Begin transform feedback section */
    #ifdef LLGL_GLEXT_TRANSFORM_FEEDBACK
    glBeginTransformFeedback(renderState_.primitiveMode);
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glBeginTransformFeedback(renderState_.primitiveMode);
    else if (HasExtension(GLExt::NV_transform_feedback))
        glBeginTransformFeedbackNV(renderState_.primitiveMode);
    #endif
}

void GLImmediateCommandBuffer::EndStreamOutput()
{
    /* End transform feedback section */
    #ifdef LLGL_GLEXT_TRANSFORM_FEEDBACK
    glEndTransformFeedback();
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glEndTransformFeedback();
    else if (HasExtension(GLExt::NV_transform_feedback))
        glEndTransformFeedbackNV();
    #endif
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type <GLintptr> to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

void GLImmediateCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    #ifdef LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        vertexOffset
    );
    #endif
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    glDrawArraysInstanced(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    #ifdef LLGL_GLEXT_BASE_INSTANCE
    glDrawArraysInstancedBaseInstance(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        firstInstance
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    #ifdef LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifdef LLGL_GLEXT_BASE_INSTANCE
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        firstInstance
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifdef LLGL_GLEXT_DRAW_INDIRECT
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawArraysIndirect(
        renderState_.drawMode,
        reinterpret_cast<const GLvoid*>(indirect)
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifdef LLGL_GLEXT_DRAW_INDIRECT
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
    #ifdef LLGL_GLEXT_MULTI_DRAW_INDIRECT
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawArraysIndirect(
            renderState_.drawMode,
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawArraysIndirect(
                renderState_.drawMode,
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifdef LLGL_GLEXT_DRAW_INDIRECT
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawElementsIndirect(
        renderState_.drawMode,
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indirect)
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifdef LLGL_GLEXT_DRAW_INDIRECT
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
    #ifdef LLGL_GLEXT_MULTI_DRAW_INDIRECT
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawElementsIndirect(
            renderState_.drawMode,
            renderState_.indexBufferDataType,
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawElementsIndirect(
                renderState_.drawMode,
                renderState_.indexBufferDataType,
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
    #endif
}

/* ----- Compute ----- */

void GLImmediateCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #ifdef LLGL_GLEXT_COMPUTE_SHADER
    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
    #endif
}

void GLImmediateCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifdef LLGL_GLEXT_COMPUTE_SHADER
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DISPATCH_INDIRECT_BUFFER, bufferGL.GetID());
    glDispatchComputeIndirect(static_cast<GLintptr>(offset));
    #endif
}

/* ----- Debugging ----- */

void GLImmediateCommandBuffer::PushDebugGroup(const char* name)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        /* Push debug group name into command stream with default ID no. */
        const GLint         maxLength       = stateMngr_->GetLimits().maxDebugNameLength;
        const GLuint        id              = 0;
        const std::size_t   actualLength    = std::strlen(name);
        const std::size_t   croppedLength   = std::min(actualLength, static_cast<std::size_t>(maxLength));

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(croppedLength), name);
    }
    #endif // /GL_KHR_debug
}

void GLImmediateCommandBuffer::PopDebugGroup()
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
        glPopDebugGroup();
    #endif // /GL_KHR_debug
}

/* ----- Extensions ----- */

void GLImmediateCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    if (stateDesc != nullptr && stateDescSize == sizeof(OpenGLDependentStateDescriptor))
    {
        stateMngr_->SetGraphicsAPIDependentState(
            *reinterpret_cast<const OpenGLDependentStateDescriptor*>(stateDesc)
        );
    }
}

/* ----- Internal ----- */

bool GLImmediateCommandBuffer::IsImmediateCmdBuffer() const
{
    return true;
}


} // /namespace LLGL



// ================================================================================
