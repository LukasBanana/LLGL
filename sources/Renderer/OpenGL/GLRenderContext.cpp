/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"
#include "GLRenderSystem.h"
#include "GLTypes.h"
#include "GLExtensions.h"
#include "../CheckedCast.h"
#include "Shader/GLShaderProgram.h"
#include "Texture/GLTexture.h"
#include "Texture/GLRenderTarget.h"
#include "Texture/GLSampler.h"
#include "RenderState/GLGraphicsPipeline.h"
#include <LLGL/Platform/NativeHandle.h>


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
    /* Setup window for the render context */
    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext);
    SetWindow(window, desc.videoMode, &windowContext);

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

void GLRenderContext::SetViewports(const std::vector<Viewport>& viewports)
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

    /* Set final state */
    stateMngr_->SetViewports(viewportsGL);
    stateMngr_->SetDepthRanges(depthRangesGL);
}

void GLRenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    /* Setup GL viewports and depth-ranges */
    std::vector<GLScissor> scissorsGL;
    scissorsGL.reserve(scissors.size());

    for (const auto& sc : scissors)
        scissorsGL.push_back({ sc.x, sc.y, sc.width, sc.height });

    /* Set final state */
    stateMngr_->SetScissors(scissorsGL);
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

void GLRenderContext::SetDrawMode(const DrawMode drawMode)
{
    renderState_.drawMode = GLTypes::Map(drawMode);
}

/* ----- Hardware Buffers ------ */

void GLRenderContext::BindVertexBuffer(VertexBuffer& vertexBuffer)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
}

void GLRenderContext::UnbindVertexBuffer()
{
    stateMngr_->BindVertexArray(0);
}

void GLRenderContext::BindIndexBuffer(IndexBuffer& indexBuffer)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    stateMngr_->BindBuffer(indexBufferGL);

    /* Store new index buffer data in global render state */
    renderState_.indexBufferDataType    = GLTypes::Map(indexBuffer.GetIndexFormat().GetDataType());
    renderState_.indexBufferStride      = indexBuffer.GetIndexFormat().GetFormatSize();
}

void GLRenderContext::UnbindIndexBuffer()
{
    stateMngr_->BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, 0);
}

void GLRenderContext::BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    stateMngr_->BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, index, constantBufferGL.hwBuffer.GetID());
}

void GLRenderContext::UnbindConstantBuffer(unsigned int index)
{
    //todo...
}

/* ----- Textures ----- */

void GLRenderContext::BindTexture(unsigned int layer, Texture& texture)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(layer);
    stateMngr_->BindTexture(textureGL);
}

void GLRenderContext::UnbindTexture(unsigned int layer)
{
    //todo...
    //stateMngr_->ActiveTexture(layer);
    //stateMngr_->BindTexture(<which target?, 0);
}

void GLRenderContext::GenerateMips(Texture& texture)
{
    /* Bind texture to active layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->BindTexture(textureGL);

    /* Generate MIP-maps and update minification filter */
    auto target = GLTypes::Map(textureGL.GetType());

    glGenerateMipmap(target);
    /*glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);*/
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

/* ----- Sampler States ----- */

void GLRenderContext::BindSampler(unsigned int layer, Sampler& sampler)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(layer, samplerGL.GetID());
}

void GLRenderContext::UnbindSampler(unsigned int layer)
{
    stateMngr_->BindSampler(layer, 0);
}

/* ----- Render Targets ----- */

void GLRenderContext::BindRenderTarget(RenderTarget& renderTarget)
{
    /* Bind framebuffer object */
    auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFrameBuffer().GetID());

    /* Notify state manager about new render target height */
    stateMngr_->NotifyRenderTargetHeight(renderTarget.GetResolution().y);

    /* Store current render target */
    boundRenderTarget_ = &renderTargetGL;
}

void GLRenderContext::UnbindRenderTarget()
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

void GLRenderContext::BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    graphicsPipelineGL.Bind(*stateMngr_);
}

/* ----- Queries ----- */

void GLRenderContext::BeginQuery(Query& query)
{
    //todo
}

void GLRenderContext::EndQuery()
{
    //todo
}

bool GLRenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    return false;//todo
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
    glDrawArraysInstancedBaseInstance(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        instanceOffset
    );
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
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        instanceOffset
    );
}

/* ----- Compute ----- */

void GLRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
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
        stateMngr_ = std::make_shared<GLStateManager>(renderSystem_);
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
