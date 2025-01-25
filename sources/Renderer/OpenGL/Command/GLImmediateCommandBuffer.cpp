/*
 * GLImmediateCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLImmediateCommandBuffer.h"
#include "GLDeferredCommandBuffer.h"
#include "GLCommandExecutor.h"
#include <LLGL/Constants.h>
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>

#include "../../TextureUtils.h"
#include "../GLSwapChain.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../Profile/GLProfile.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLEmulatedSampler.h"
#include "../Texture/GLRenderTarget.h"
#include "../Texture/GLMipGenerator.h"
#include "../Texture/GLFramebufferCapture.h"

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferWithXFB.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLGraphicsPSO.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <cstring> // std::strlen

#include <LLGL/Backend/OpenGL/NativeCommand.h>


namespace LLGL
{


GLImmediateCommandBuffer::GLImmediateCommandBuffer() :
    stateMngr_ { &(GLStateManager::Get()) }
{
}

/* ----- Encoding ----- */

void GLImmediateCommandBuffer::Begin()
{
    stateMngr_ = &(GLStateManager::Get());
    ResetRenderState();
}

void GLImmediateCommandBuffer::End()
{
    // dummy
}

void GLImmediateCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    auto& cmdBufferGL = LLGL_CAST(const GLCommandBuffer&, secondaryCommandBuffer);
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
    const TextureSubresource zeroBasedSubresource{ 0, srcRegion.subresource.numArrayLayers, 0, 1 };
    srcTextureGL.CopyImageToBuffer(
        srcRegion,
        dstBufferGL.GetID(),
        static_cast<GLintptr>(dstOffset),
        static_cast<GLsizei>(GetMemoryFootprint(srcTextureGL.GetType(), srcTextureGL.GetFormat(), srcRegion.extent, zeroBasedSubresource)),
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
    if (fillSize == LLGL_WHOLE_SIZE)
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
    const TextureSubresource zeroBasedSubresource{ 0, dstRegion.subresource.numArrayLayers, 0, 1 };
    dstTextureGL.CopyImageFromBuffer(
        dstRegion,
        srcBufferGL.GetID(),
        static_cast<GLintptr>(srcOffset),
        static_cast<GLsizei>(GetMemoryFootprint(dstTextureGL.GetType(), dstTextureGL.GetFormat(), dstRegion.extent, zeroBasedSubresource)),
        static_cast<GLint>(rowStride),
        static_cast<GLint>(rowStride > 0 ? layerStride / rowStride : 0)
    );
}

void GLImmediateCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    if (dstRegion.extent.depth != 1)
        return /*GL_INVALID_VALUE*/;

    auto& dstTextureGL = LLGL_CAST(GLTexture&, dstTexture);
    GLFramebufferCapture::Get().CaptureFramebuffer(
        *stateMngr_,
        dstTextureGL,
        static_cast<GLint>(dstRegion.subresource.baseMipLevel),
        CalcTextureOffset(dstTexture.GetType(), dstRegion.offset, dstRegion.subresource.baseArrayLayer),
        srcOffset,
        Extent2D{ dstRegion.extent.width, dstRegion.extent.height }
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
    const GLViewport viewportGL{ viewport.x, viewport.y, viewport.width, viewport.height };
    const GLDepthRange depthRangeGL{ viewport.minDepth, viewport.maxDepth };

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

    for_range(i, count)
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
    const GLScissor scissorGL{ scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLImmediateCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    GLScissor scissorsGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Setup GL scissors */
    auto count = static_cast<GLsizei>(std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

    for_range(i, count)
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

/* ----- Input Assembly ------ */

void GLImmediateCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferGL = LLGL_CAST(GLBufferWithVAO&, buffer);
        vertexBufferGL.GetVertexArray()->Bind(*stateMngr_);

        #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
        /* Store ID to transform feedback object */
        if ((buffer.GetBindFlags() & BindFlags::StreamOutputBuffer) != 0)
        {
            auto& streamOutputBufferGL = LLGL_CAST(GLBufferWithXFB&, vertexBufferGL);
            SetTransformFeedback(streamOutputBufferGL);
        }
        #endif // /LLGL_GLEXT_TRNASFORM_FEEDBACK2
    }
}

void GLImmediateCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferArrayGL = LLGL_CAST(GLBufferArrayWithVAO&, bufferArray);
        vertexBufferArrayGL.GetVertexArray()->Bind(*stateMngr_);
    }
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID(), bufferGL.IsIndexType16Bits());
    SetIndexFormat(bufferGL.IsIndexType16Bits(), 0);
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    const bool indexType16Bits = (format == Format::R16UInt);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID(), indexType16Bits);
    SetIndexFormat(indexType16Bits, offset);
}

/* ----- Resource Heaps ----- */

void GLImmediateCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto& resourceHeapGL = LLGL_CAST(GLResourceHeap&, resourceHeap);
    resourceHeapGL.Bind(*stateMngr_, descriptorSet, GetBoundPipelineState()->GetBufferInterfaceMap());
    #if LLGL_GLEXT_MEMORY_BARRIERS
    InvalidateMemoryBarriers(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT); //TODO: find optimal bitmask from resource heap
    #endif
}

void GLImmediateCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    auto* pipelineLayoutGL = GetBoundPipelineLayout();
    if (pipelineLayoutGL == nullptr)
        return /*GL_INVALID_VALUE*/;

    const auto& bindingList = pipelineLayoutGL->GetBindings();
    if (!(descriptor < bindingList.size()))
        return /*GL_INVALID_INDEX*/;

    const GLPipelineResourceBinding& binding = bindingList[descriptor];
    if (binding.combiners > 0)
    {
        /* Bind resource at one or more slots for combined texture-samplers */
        const std::vector<GLuint>& combinedSamplerSlots = pipelineLayoutGL->GetCombinedSamplerSlots();
        BindCombinedResource(binding.type, &(combinedSamplerSlots[binding.slot]), binding.combiners, resource);
    }
    else
    {
        /* Bind resource at explicit binding slot */
        BindResource(binding.type, binding.slot, binding.ssboIndex, resource);
    }
}

void GLImmediateCommandBuffer::ResourceBarrier(
    std::uint32_t       numBuffers,
    Buffer* const *     buffers,
    std::uint32_t       numTextures,
    Texture* const *    textures)
{
    #if LLGL_GLEXT_MEMORY_BARRIERS
    InvalidateMemoryBarriersForResources(numBuffers, buffers, numTextures, textures);
    #endif
}

// private
void GLImmediateCommandBuffer::BindResource(GLResourceType type, GLuint slot, std::uint32_t descriptor, Resource& resource)
{
    switch (type)
    {
        case GLResourceType_Invalid:
        case GLResourceType_End:
        {
            // ignore
        }
        break;

        case GLResourceType_UBO:
        {
            auto& bufferGL = LLGL_CAST(GLBuffer&, resource);
            stateMngr_->BindBufferBase(GLBufferTarget::UniformBuffer, slot, bufferGL.GetID());
        }
        break;

        case GLResourceType_Buffer:
        {
            auto& bufferGL = LLGL_CAST(GLBuffer&, resource);

            /* Lookup whether this is an SSBO, sampler buffer, or image buffer in buffer interface map */
            const GLShaderBufferInterfaceMap* bufferInterfaceMap = GetBoundPipelineState()->GetBufferInterfaceMap();
            GLBufferInterface bufferInterface = bufferInterfaceMap->GetDynamicInterfaces()[descriptor];
            switch (bufferInterface)
            {
                case GLBufferInterface_SSBO:
                {
                    stateMngr_->BindBufferBase(GLBufferTarget::ShaderStorageBuffer, slot, bufferGL.GetID());
                    #if LLGL_GLEXT_MEMORY_BARRIERS
                    InvalidateMemoryBarriersForStorageResource(bufferGL.GetBindFlags(), GL_SHADER_STORAGE_BARRIER_BIT);
                    #endif
                }
                break;

                case GLBufferInterface_Sampler:
                {
                    stateMngr_->BindTexture(slot, GLTextureTarget::TextureBuffer, bufferGL.GetTexID());
                    #if LLGL_GLEXT_MEMORY_BARRIERS
                    InvalidateMemoryBarriersForStorageResource(bufferGL.GetBindFlags(), GL_TEXTURE_FETCH_BARRIER_BIT);
                    #endif
                }
                break;

                case GLBufferInterface_Image:
                {
                    stateMngr_->BindImageTexture(slot, 0, bufferGL.GetTexGLInternalFormat(), bufferGL.GetTexID());
                    #if LLGL_GLEXT_MEMORY_BARRIERS
                    InvalidateMemoryBarriersForStorageResource(bufferGL.GetBindFlags(), GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    #endif
                }
                break;
            }
        }
        break;

        case GLResourceType_Texture:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);
            stateMngr_->BindGLTexture(slot, textureGL);
            #if LLGL_GLEXT_MEMORY_BARRIERS
            InvalidateMemoryBarriersForStorageResource(textureGL.GetBindFlags(), GL_TEXTURE_FETCH_BARRIER_BIT);
            #endif
        }
        break;

        case GLResourceType_Image:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);
            stateMngr_->BindImageTexture(slot, 0, textureGL.GetGLInternalFormat(), textureGL.GetID());
            #if LLGL_GLEXT_MEMORY_BARRIERS
            InvalidateMemoryBarriersForStorageResource(textureGL.GetBindFlags(), GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            #endif
        }
        break;

        case GLResourceType_Sampler:
        {
            auto& samplerGL = LLGL_CAST(GLSampler&, resource);
            stateMngr_->BindSampler(slot, samplerGL.GetID());
        }
        break;

        case GLResourceType_EmulatedSampler:
        {
            auto& emulatedSamplerGL = LLGL_CAST(GLEmulatedSampler&, resource);
            stateMngr_->BindEmulatedSampler(slot, emulatedSamplerGL);
        }
        break;
    }
}

// private
void GLImmediateCommandBuffer::BindCombinedResource(GLResourceType type, const GLuint* slots, std::uint32_t numSlots, Resource& resource)
{
    switch (type)
    {
        case GLResourceType_Texture:
        {
            auto& textureGL = LLGL_CAST(GLTexture&, resource);
            for_range(i, numSlots)
                stateMngr_->BindGLTexture(slots[i], textureGL);
            #if LLGL_GLEXT_MEMORY_BARRIERS
            if ((textureGL.GetBindFlags() & BindFlags::Storage) != 0)
                InvalidateMemoryBarriers(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            #endif
        }
        break;

        case GLResourceType_Sampler:
        {
            auto& samplerGL = LLGL_CAST(GLSampler&, resource);
            for_range(i, numSlots)
                stateMngr_->BindSampler(slots[i], samplerGL.GetID());
        }
        break;

        case GLResourceType_EmulatedSampler:
        {
            auto& emulatedSamplerGL = LLGL_CAST(GLEmulatedSampler&, resource);
            for_range(i, numSlots)
                stateMngr_->BindEmulatedSampler(slots[i], emulatedSamplerGL);
        }
        break;

        default:
        {
            // dummy - combined resource bindings are only available to textures and samplers
        }
        break;
    }
}

/* ----- Render Passes ----- */

void GLImmediateCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    /* Bind render target and update state manager if GL context has switched */
    auto nextStateMngr = stateMngr_;
    stateMngr_->BindRenderTarget(renderTarget, &nextStateMngr);
    stateMngr_ = nextStateMngr;

    /* Clear render target attachments with render pass */
    if (renderPass != nullptr)
    {
        auto renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
        stateMngr_->ClearAttachmentsWithRenderPass(*renderPassGL, numClearValues, clearValues);
    }
}

void GLImmediateCommandBuffer::EndRenderPass()
{
    /* Resolve previously bound render target */
    if (GLRenderTarget* renderTarget = stateMngr_->GetBoundRenderTarget())
        renderTarget->ResolveMultisampled(*stateMngr_);
}

void GLImmediateCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if ((flags & ClearFlags::Color) != 0)
    {
        glClearColor(
            clearValue.color[0],
            clearValue.color[1],
            clearValue.color[2],
            clearValue.color[3]
        );
    }

    if ((flags & ClearFlags::Depth) != 0)
        GLProfile::ClearDepth(static_cast<GLclamp_t>(clearValue.depth));

    if ((flags & ClearFlags::Stencil) != 0)
        glClearStencil(static_cast<GLint>(clearValue.stencil));

    stateMngr_->Clear(flags);
}

void GLImmediateCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    stateMngr_->ClearBuffers(numAttachments, attachments);
}

/* ----- Pipeline States ----- */

void GLImmediateCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Bind graphics pipeline render states */
    auto& pipelineStateGL = LLGL_CAST(GLPipelineState&, pipelineState);
    pipelineStateGL.Bind(*stateMngr_);
    SetPipelineRenderState(pipelineStateGL);
}

void GLImmediateCommandBuffer::SetBlendFactor(const float color[4])
{
    stateMngr_->SetBlendColor(color);
}

void GLImmediateCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    stateMngr_->SetStencilRef(static_cast<GLint>(reference), GLTypes::Map(stencilFace));
}

void GLImmediateCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    /* Data size must be a multiple of 4 bytes */
    if (dataSize == 0 || dataSize % 4 != 0 || data == nullptr)
        return /*GL_INVALID_VALUE*/;

    auto* boundPipelineState = GetBoundPipelineState();
    if (boundPipelineState == nullptr)
        return /*GL_INVALID_VALUE*/;

    const std::uint32_t dataSizeInWords = dataSize / 4;
    const auto& uniformMap = boundPipelineState->GetUniformMap();

    for (auto words = static_cast<const std::uint32_t*>(data), wordsEnd = words + dataSizeInWords; words != wordsEnd; ++first)
    {
        if (first >= uniformMap.size())
            return /*GL_INVALID_INDEX*/;

        const auto& uniform = uniformMap[first];
        GLSetUniform(uniform.type, uniform.location, uniform.count, words);

        words += uniform.wordSize;
    }
}

/* ----- Queries ----- */

void GLImmediateCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.Begin(query);
}

void GLImmediateCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t /*query*/)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.End();
}

void GLImmediateCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    #if LLGL_GLEXT_CONDITIONAL_RENDER
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    glBeginConditionalRender(queryHeapGL.GetID(query), GLTypes::Map(mode));
    #endif
}

void GLImmediateCommandBuffer::EndRenderCondition()
{
    #if LLGL_GLEXT_CONDITIONAL_RENDER
    glEndConditionalRender();
    #endif
}

/* ----- Stream Output ------ */

void GLImmediateCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    #if !LLGL_GL_ENABLE_OPENGL2X

    /* Bind transform feedback buffers */
    GLuint soTargets[LLGL_MAX_NUM_SO_BUFFERS];
    numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

    if (numBuffers > 0)
    {
        auto* bufferWithXfbGL = LLGL_CAST(GLBufferWithXFB*, buffers[0]);
        GLBufferWithXFB::BeginTransformFeedback(*stateMngr_, *bufferWithXfbGL, GetPrimitiveMode());
    }

    for_range(i, numBuffers)
    {
        auto* bufferGL = LLGL_CAST(GLBuffer*, buffers[i]);
        soTargets[i] = bufferGL->GetID();
    }

    stateMngr_->BindBuffersBase(GLBufferTarget::TransformFeedbackBuffer, 0, static_cast<GLsizei>(numBuffers), soTargets);

    /* Begin transform feedback section */
    #if LLGL_GLEXT_TRANSFORM_FEEDBACK
    glBeginTransformFeedback(GetPrimitiveMode());
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glBeginTransformFeedback(GetPrimitiveMode());
    else if (HasExtension(GLExt::NV_transform_feedback))
        glBeginTransformFeedbackNV(GetPrimitiveMode());
    #endif

    #endif // /!LLGL_GL_ENABLE_OPENGL2X
}

void GLImmediateCommandBuffer::EndStreamOutput()
{
    #if !LLGL_GL_ENABLE_OPENGL2X

    /* End transform feedback section */
    #if LLGL_GLEXT_TRANSFORM_FEEDBACK
    glEndTransformFeedback();
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glEndTransformFeedback();
    else if (HasExtension(GLExt::NV_transform_feedback))
        glEndTransformFeedbackNV();
    #endif

    GLBufferWithXFB::EndTransformFeedback(*stateMngr_);

    #endif // /!LLGL_GL_ENABLE_OPENGL2X
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type <GLintptr> to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

#if LLGL_GLEXT_MEMORY_BARRIERS
#   define LLGL_FLUSH_MEMORY_BARRIERS() \
        if (GLbitfield barriers = FlushAndGetMemoryBarriers()) { glMemoryBarrier(barriers); }
#else
#   define LLGL_FLUSH_MEMORY_BARRIERS()
#endif // /LLGL_GLEXT_MEMORY_BARRIERS

void GLImmediateCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawArrays(
        GetDrawMode(),
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawElements(
        GetDrawMode(),
        static_cast<GLsizei>(numIndices),
        GetIndexType(),
        GetIndicesOffset(firstIndex)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    #if LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawElementsBaseVertex(
        GetDrawMode(),
        static_cast<GLsizei>(numIndices),
        GetIndexType(),
        GetIndicesOffset(firstIndex),
        vertexOffset
    );
    #endif // /LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    #if LLGL_GLEXT_DRAW_INSTANCED
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawArraysInstanced(
        GetDrawMode(),
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
    #endif
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    #if LLGL_GLEXT_BASE_INSTANCE
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawArraysInstancedBaseInstance(
        GetDrawMode(),
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        firstInstance
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    #if LLGL_GLEXT_DRAW_INSTANCED
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawElementsInstanced(
        GetDrawMode(),
        static_cast<GLsizei>(numIndices),
        GetIndexType(),
        GetIndicesOffset(firstIndex),
        static_cast<GLsizei>(numInstances)
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    #if LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawElementsInstancedBaseVertex(
        GetDrawMode(),
        static_cast<GLsizei>(numIndices),
        GetIndexType(),
        GetIndicesOffset(firstIndex),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
    #endif // /LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #if LLGL_GLEXT_BASE_INSTANCE
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDrawElementsInstancedBaseVertexBaseInstance(
        GetDrawMode(),
        static_cast<GLsizei>(numIndices),
        GetIndexType(),
        GetIndicesOffset(firstIndex),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        firstInstance
    );
    #endif
}

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    #if LLGL_GLEXT_DRAW_INDIRECT
    LLGL_FLUSH_MEMORY_BARRIERS();

    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DrawIndirectBuffer, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawArraysIndirect(
        GetDrawMode(),
        reinterpret_cast<const GLvoid*>(indirect)
    );
    #endif // /LLGL_GLEXT_DRAW_INDIRECT
}

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #if LLGL_GLEXT_DRAW_INDIRECT
    LLGL_FLUSH_MEMORY_BARRIERS();

    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DrawIndirectBuffer, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
    #if LLGL_GLEXT_MULTI_DRAW_INDIRECT
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawArraysIndirect(
            GetDrawMode(),
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif // /LLGL_GLEXT_MULTI_DRAW_INDIRECT
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawArraysIndirect(
                GetDrawMode(),
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
    #endif // /LLGL_GLEXT_DRAW_INDIRECT
}

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    #if LLGL_GLEXT_DRAW_INDIRECT
    LLGL_FLUSH_MEMORY_BARRIERS();

    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DrawIndirectBuffer, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawElementsIndirect(
        GetDrawMode(),
        GetIndexType(),
        reinterpret_cast<const GLvoid*>(indirect)
    );
    #endif // /LLGL_GLEXT_DRAW_INDIRECT
}

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #if LLGL_GLEXT_DRAW_INDIRECT
    LLGL_FLUSH_MEMORY_BARRIERS();

    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DrawIndirectBuffer, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
    #if LLGL_GLEXT_MULTI_DRAW_INDIRECT
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawElementsIndirect(
            GetDrawMode(),
            GetIndexType(),
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif // /LLGL_GLEXT_MULTI_DRAW_INDIRECT
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawElementsIndirect(
                GetDrawMode(),
                GetIndexType(),
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
    #endif // /LLGL_GLEXT_DRAW_INDIRECT
}

void GLImmediateCommandBuffer::DrawStreamOutput()
{
    if (GLBufferWithXFB* bufferWithXfbGL = GetRenderState().boundBufferWithFxb)
    {
        LLGL_FLUSH_MEMORY_BARRIERS();
        #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
        if (HasExtension(GLExt::ARB_transform_feedback2))
        {
            /* Draw primitives with internal number of vertices */
            glDrawTransformFeedback(GetDrawMode(), bufferWithXfbGL->GetTransformFeedbackID());
        }
        else
        #endif // /LLGL_GLEXT_TRNASFORM_FEEDBACK2
        {
            /* Draw primitives with the queried number of vertices */
            glDrawArrays(GetDrawMode(), 0, bufferWithXfbGL->QueryVertexCount());
        }
    }
}

/* ----- Compute ----- */

void GLImmediateCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #if LLGL_GLEXT_COMPUTE_SHADER
    LLGL_FLUSH_MEMORY_BARRIERS();
    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
    #endif
}

void GLImmediateCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    #if LLGL_GLEXT_COMPUTE_SHADER
    LLGL_FLUSH_MEMORY_BARRIERS();
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DispatchIndirectBuffer, bufferGL.GetID());
    glDispatchComputeIndirect(static_cast<GLintptr>(offset));
    #endif
}

/* ----- Debugging ----- */

void GLImmediateCommandBuffer::PushDebugGroup(const char* name)
{
    #if LLGL_GLEXT_DEBUG
    if (HasExtension(GLExt::KHR_debug))
    {
        /* Push debug group name into command stream with default ID no. */
        const GLint         maxLength       = stateMngr_->GetLimits().maxDebugNameLength;
        const GLuint        id              = 0;
        const std::size_t   actualLength    = std::strlen(name);
        const std::size_t   croppedLength   = std::min(actualLength, static_cast<std::size_t>(maxLength));

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(croppedLength), name);
    }
    #endif // /LLGL_GLEXT_DEBUG
}

void GLImmediateCommandBuffer::PopDebugGroup()
{
    #if LLGL_GLEXT_DEBUG
    if (HasExtension(GLExt::KHR_debug))
        glPopDebugGroup();
    #endif // /LLGL_GLEXT_DEBUG
}

/* ----- Extensions ----- */

void GLImmediateCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    if (nativeCommand != nullptr && nativeCommandSize == sizeof(OpenGL::NativeCommand))
    {
        const auto* nativeCommandGL = static_cast<const OpenGL::NativeCommand*>(nativeCommand);
        ExecuteNativeGLCommand(*nativeCommandGL, *stateMngr_);
    }
}


/*
 * ======= Internal: =======
 */

bool GLImmediateCommandBuffer::IsImmediateCmdBuffer() const
{
    return true;
}


} // /namespace LLGL



// ================================================================================
