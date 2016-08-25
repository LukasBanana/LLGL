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

std::map<RendererInfo, std::string> GLRenderContext::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    std::vector<std::pair<RendererInfo, GLenum>> entries
    {{
        { RendererInfo::Version,                GL_VERSION                  },
        { RendererInfo::Vendor,                 GL_VENDOR                   },
        { RendererInfo::Hardware,               GL_RENDERER                 },
        { RendererInfo::ShadingLanguageVersion, GL_SHADING_LANGUAGE_VERSION },
    }};
    
    for (const auto& entry : entries)
    {
        auto bytes = glGetString(entry.second);
        if (bytes)
            info[entry.first] = std::string(reinterpret_cast<const char*>(bytes));
    }

    return info;
}

RenderingCaps GLRenderContext::QueryRenderingCaps() const
{
    RenderingCaps caps;

    /* Query all boolean capabilies by their respective OpenGL extension */
    caps.hasRenderTargets       = HasExtension("GL_ARB_framebuffer_object");
    caps.has3DTextures          = HasExtension("GL_EXT_texture3D");
    caps.hasCubeTextures        = HasExtension("GL_ARB_texture_cube_map");
    caps.hasTextureArrays       = HasExtension("GL_EXT_texture_array");
    caps.hasCubeTextureArrays   = HasExtension("GL_ARB_texture_cube_map_array");
    caps.hasConstantBuffers     = HasExtension("GL_ARB_uniform_buffer_object");
    caps.hasStorageBuffers      = HasExtension("GL_ARB_shader_storage_buffer_object");
    caps.hasUniforms            = HasExtension("GL_ARB_shader_objects");
    caps.hasGeometryShaders     = HasExtension("GL_ARB_geometry_shader4");
    caps.hasTessellationShaders = HasExtension("GL_ARB_tessellation_shader");
    caps.hasComputeShaders      = HasExtension("GL_ARB_compute_shader");
    caps.hasInstancing          = HasExtension("GL_ARB_draw_instanced");
    caps.hasOffsetInstancing    = HasExtension("GL_ARB_base_instance");
    caps.hasViewportArrays      = HasExtension("GL_ARB_viewport_array");

    /* Query integral attributes */
    auto GetUInt = [](GLenum param)
    {
        GLint attr = 0;
        glGetIntegerv(param, &attr);
        return static_cast<unsigned int>(attr);
    };

    caps.maxNumTextureArrayLayers       = GetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    caps.maxNumRenderTargetAttachments  = GetUInt(GL_MAX_DRAW_BUFFERS);
    caps.maxConstantBufferSize          = GetUInt(GL_MAX_UNIFORM_BLOCK_SIZE);

    /* Query maximum texture dimensions */
    GLint querySizeBase = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &querySizeBase);

    /* Generate proxy texture */
    GLuint proxyTex = 0;
    glGenTextures(1, &proxyTex);

    /* --- Query 1D texture max size --- */
    auto querySize = querySizeBase;

    while (caps.max1DTextureSize == 0 && querySize > 0)
    {
        glTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &(caps.max1DTextureSize));
        querySize /= 2;
    }

    /* Query 2D texture max size */
    querySize = querySizeBase;

    while (caps.max2DTextureSize == 0 && querySize > 0)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &(caps.max2DTextureSize));
        querySize /= 2;
    }

    /* Query 3D texture max size */
    if (caps.has3DTextures)
    {
        querySize = querySizeBase;

        while (caps.max3DTextureSize == 0 && querySize > 0)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA, querySize, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &(caps.max3DTextureSize));
            querySize /= 2;
        }
    }

    /* Query cube texture max size */
    if (caps.hasCubeTextures)
    {
        querySize = querySizeBase;

        while (caps.maxCubeTextureSize == 0 && querySize > 0)
        {
            glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &(caps.maxCubeTextureSize));
            querySize /= 2;
        }
    }

    /* Delete temporary proxy texture */
    glDeleteTextures(1, &proxyTex);

    return caps;
}

ShadingLanguage GLRenderContext::QueryShadingLanguage() const
{
    /* Derive shading language version by OpenGL version */
    GLint major = 0, minor = 0;
    QueryGLVerion(major, minor);

    auto IsVer = [major, minor](int maj, int min)
    {
        return (major == maj && minor == min);
    };

    if (IsVer(2, 0)) return ShadingLanguage::GLSL_110;
    if (IsVer(2, 1)) return ShadingLanguage::GLSL_120;
    if (IsVer(3, 0)) return ShadingLanguage::GLSL_130;
    if (IsVer(3, 1)) return ShadingLanguage::GLSL_140;
    if (IsVer(3, 2)) return ShadingLanguage::GLSL_150;
    if (IsVer(3, 3)) return ShadingLanguage::GLSL_330;
    if (IsVer(4, 0)) return ShadingLanguage::GLSL_400;
    if (IsVer(4, 1)) return ShadingLanguage::GLSL_410;
    if (IsVer(4, 2)) return ShadingLanguage::GLSL_420;
    if (IsVer(4, 3)) return ShadingLanguage::GLSL_430;
    if (IsVer(4, 4)) return ShadingLanguage::GLSL_440;
    if (IsVer(4, 5)) return ShadingLanguage::GLSL_450;

    return ShadingLanguage::GLSL_110;
}

/* ----- Configuration ----- */

void GLRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        contextHeight_ = videoModeDesc.resolution.y;
        stateMngr_->MakeCurrentInfo(*this);
        RenderContext::SetVideoMode(videoModeDesc);
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

void GLRenderContext::BindTexture(Texture& texture, unsigned int layer)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(layer);
    stateMngr_->BindTexture(textureGL);
}

void GLRenderContext::UnbindTexture(unsigned int layer)
{
    //todo...
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

/* ----- Render Targets ----- */

void GLRenderContext::BindRenderTarget(RenderTarget& renderTarget)
{
    auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFrameBuffer().GetID());
}

void GLRenderContext::UnbindRenderTarget()
{
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);
}

/* ----- Pipeline States ----- */

void GLRenderContext::BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    graphicsPipelineGL.Bind(*stateMngr_);
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
        stateMngr_ = std::make_shared<GLStateManager>();
    }

    /* Notify state manager about the current render context */
    stateMngr_->MakeCurrentInfo(*this);
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

void GLRenderContext::QueryGLVerion(GLint& major, GLint& minor) const
{
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
}

bool GLRenderContext::HasExtension(const std::string& name) const
{
    return renderSystem_.HasExtension(name);
}


} // /namespace LLGL



// ================================================================================
