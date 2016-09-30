/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"
#include "GLRenderSystem.h"
#include "GLTypes.h"
#include "Ext/GLExtensions.h"
#include "../Assertion.h"
#include "../CheckedCast.h"
#include "Shader/GLShaderProgram.h"
#include "Texture/GLTexture.h"
#include "Texture/GLRenderTarget.h"
#include "Texture/GLSampler.h"
#include "Buffer/GLVertexBuffer_.h"
#include "Buffer/GLIndexBuffer_.h"
#include "RenderState/GLGraphicsPipeline.h"

#ifndef __APPLE__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


GLRenderContext::GLRenderContext(
    GLRenderSystem& renderSystem,
    RenderContextDescriptor desc,
    const std::shared_ptr<Window>& window,
    GLRenderContext* sharedRenderContext) :
        renderSystem_   ( renderSystem                ),
        desc_           ( desc                        ),
        contextHeight_  ( desc.videoMode.resolution.y )
{
    #ifndef __APPLE__
    /* Setup window for the render context */
    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext);
    SetWindow(window, desc.videoMode, &windowContext);
    #endif

    /* Acquire state manager to efficiently change render states */
    AcquireStateManager(sharedRenderContext);

    /* Create platform dependent OpenGL context */
    CreateContext(sharedRenderContext);

    /* Initialize render states for the first time */
    if (!sharedRenderContext)
        InitRenderStates();
}

GLRenderContext::~GLRenderContext()
{
    DeleteContext();
}

/* ----- Configuration ----- */

void GLRenderContext::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    stateMngr_->SetGraphicsAPIDependentState(state);
}

void GLRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        /* Update context height */
        contextHeight_ = videoModeDesc.resolution.y;
        stateMngr_->NotifyRenderTargetHeight(contextHeight_);

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);
    }
}

void GLRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        SetupVsyncInterval();
    }
}

void GLRenderContext::SetViewport(const Viewport& viewport)
{
    /* Setup GL viewport and depth-range */
    GLViewport viewportGL { viewport.x, viewport.y, viewport.width, viewport.height };
    GLDepthRange depthRangeGL { viewport.minDepth, viewport.maxDepth };

    /* Set final state */
    stateMngr_->SetViewport(viewportGL);
    stateMngr_->SetDepthRange(depthRangeGL);
}

void GLRenderContext::SetViewportArray(const std::vector<Viewport>& viewports)
{
    /* Setup GL viewports and depth-ranges */
    std::vector<GLViewport> viewportsGL;
    viewportsGL.reserve(viewports.size());

    std::vector<GLDepthRange> depthRangesGL;
    depthRangesGL.reserve(viewports.size());

    for (const auto& vp : viewports)
    {
        viewportsGL.push_back({ vp.x, vp.y, vp.width, vp.height });
        depthRangesGL.push_back({ static_cast<GLdouble>(vp.minDepth), static_cast<GLdouble>(vp.maxDepth) });
    }

    /* Submit viewports and depth-ranges to state manager */
    stateMngr_->SetViewportArray(std::move(viewportsGL));
    stateMngr_->SetDepthRangeArray(std::move(depthRangesGL));
}

void GLRenderContext::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL { scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLRenderContext::SetScissorArray(const std::vector<Scissor>& scissors)
{
    /* Setup GL scissors */
    std::vector<GLScissor> scissorsGL;
    scissorsGL.reserve(scissors.size());

    for (const auto& sc : scissors)
        scissorsGL.push_back({ sc.x, sc.y, sc.width, sc.height });

    /* Submit scissors to state manager */
    stateMngr_->SetScissorArray(std::move(scissorsGL));
}

void GLRenderContext::SetClearColor(const ColorRGBAf& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void GLRenderContext::SetClearDepth(float depth)
{
    glClearDepth(depth);
}

void GLRenderContext::SetClearStencil(int stencil)
{
    glClearStencil(stencil);
}

void GLRenderContext::ClearBuffers(long flags)
{
    GLbitfield mask = 0;

    if ((flags & ClearBuffersFlags::Color) != 0)
    {
        //stateMngr_->SetColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    
    if ((flags & ClearBuffersFlags::Depth) != 0)
    {
        stateMngr_->SetDepthMask(GL_TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearBuffersFlags::Stencil) != 0)
    {
        //stateMngr_->SetStencilMask(GL_TRUE);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

/* ----- Hardware Buffers ------ */

void GLRenderContext::SetVertexBuffer(Buffer& buffer)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer_&, buffer);
    stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
}

void GLRenderContext::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer_&, buffer);
    stateMngr_->BindBuffer(indexBufferGL);

    /* Store new index buffer data in global render state */
    const auto& format = indexBufferGL.GetIndexFormat();
    renderState_.indexBufferDataType    = GLTypes::Map(format.GetDataType());
    renderState_.indexBufferStride      = format.GetFormatSize();
}

void GLRenderContext::SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    /* Bind constant buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, slot, bufferGL.GetID());
}

void GLRenderContext::SetStorageBuffer(Buffer& buffer, unsigned int slot)
{
    /* Bind storage buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(GLBufferTarget::SHADER_STORAGE_BUFFER, slot, bufferGL.GetID());
}

void* GLRenderContext::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    /* Get, bind, and map buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(bufferGL);
    return bufferGL.MapBuffer(GLTypes::Map(access));
}

void GLRenderContext::UnmapBuffer(Buffer& buffer)
{
    /* Get, bind, and unmap buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(bufferGL);
    bufferGL.UnmapBuffer();
}

/* ----- Textures ----- */

void GLRenderContext::SetTexture(Texture& texture, unsigned int slot, long /*shaderStageFlags*/)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

/* ----- Sampler States ----- */

void GLRenderContext::SetSampler(Sampler& sampler, unsigned int slot, long /*shaderStageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

/* ----- Render Targets ----- */

void GLRenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    /* Bind framebuffer object */
    auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFrameBuffer().GetID());

    /* Notify state manager about new render target height */
    stateMngr_->NotifyRenderTargetHeight(renderTarget.GetResolution().y);

    /* Store current render target */
    boundRenderTarget_ = &renderTargetGL;
}

void GLRenderContext::UnsetRenderTarget()
{
    /* Blit previously bound render target (in case mutli-sampling is used) */
    if (boundRenderTarget_)
        boundRenderTarget_->BlitOntoFrameBuffer();
        //boundRenderTarget_->BlitOntoScreen(0);//!!!

    /* Unbind framebuffer object */
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);

    /* Notify state manager about new render target (the default frame buffer) height */
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);

    /* Reset reference to render target */
    boundRenderTarget_ = nullptr;
}

/* ----- Pipeline States ----- */

void GLRenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    renderState_.drawMode = graphicsPipelineGL.GetDrawMode();
    graphicsPipelineGL.Bind(*stateMngr_);
}

void GLRenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineGL = LLGL_CAST(GLComputePipeline&, computePipeline);
    computePipelineGL.Bind(*stateMngr_);
}

/* ----- Queries ----- */

void GLRenderContext::BeginQuery(Query& query)
{
    /* Begin query with internal target */
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glBeginQuery(queryGL.GetTarget(), queryGL.GetID());
}

void GLRenderContext::EndQuery(Query& query)
{
    /* Begin query with internal target */
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glEndQuery(queryGL.GetTarget());
}

bool GLRenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);

    /* Check if query result is available */
    GLint available = 0;
    glGetQueryObjectiv(queryGL.GetID(), GL_QUERY_RESULT_AVAILABLE, &available);

    if (available != GL_FALSE)
    {
        /* Get query result either with 64-bit or 32-bit version */
        //if (glGetQueryObjectui64v)
            glGetQueryObjectui64v(queryGL.GetID(), GL_QUERY_RESULT, &result);
        /*else
        {
            GLuint result32 = 0;
            glGetQueryObjectuiv(queryGL.GetID(), GL_QUERY_RESULT, &result32);
            result = result32;
        }*/
        return true;
    }

    return false;
}

void GLRenderContext::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glBeginConditionalRender(queryGL.GetID(), GLTypes::Map(mode));
}

void GLRenderContext::EndRenderCondition()
{
    glEndConditionalRender();
}

/* ----- Drawing ----- */

void GLRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride))
    );
}

void GLRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        vertexOffset
    );
}

void GLRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    glDrawArraysInstanced(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    #ifndef __APPLE__
    glDrawArraysInstancedBaseInstance(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        instanceOffset
    );
    #endif
}

void GLRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances)
    );
}

void GLRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
}

void GLRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    #ifndef __APPLE__
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        instanceOffset
    );
    #endif
}

/* ----- Compute ----- */

void GLRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    #ifndef __APPLE__
    glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
    #endif
}

/* ----- Misc ----- */

void GLRenderContext::SyncGPU()
{
    glFinish();
}


/*
 * ======= Private: =======
 */

void GLRenderContext::AcquireStateManager(GLRenderContext* sharedRenderContext)
{
    if (sharedRenderContext)
    {
        /* Share state manager with shared render context */
        stateMngr_ = sharedRenderContext->stateMngr_;
    }
    else
    {
        /* Create a new shared state manager */
        stateMngr_ = std::make_shared<GLStateManager>();
    }

    /* Notify state manager about the current render context */
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);
}

void GLRenderContext::InitRenderStates()
{
    /* Initialize state manager */
    stateMngr_->Reset();

    /* Setup default render states to be uniform between render systems */
    stateMngr_->Enable(GLState::TEXTURE_CUBE_MAP_SEAMLESS); // D3D10+ has this per default
    stateMngr_->SetFrontFace(GL_CW);                        // D3D10+ uses clock-wise vertex winding per default

    /*
    Set pixel storage to byte-alignment (default is word-alignment).
    This is required so that texture formats like RGB (which is not word-aligned) can be used.
    */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


} // /namespace LLGL



// ================================================================================
