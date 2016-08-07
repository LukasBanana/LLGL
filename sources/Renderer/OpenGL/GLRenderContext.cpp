/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"
#include "GLTypeConversion.h"
#include "GLExtensions.h"
#include "../CheckedCast.h"

#include "Shader/GLVertexShader.h"
#include "Shader/GLFragmentShader.h"
#include "Shader/GLGeometryShader.h"
#include "Shader/GLTessControlShader.h"
#include "Shader/GLTessEvaluationShader.h"
#include "Shader/GLComputeShader.h"
#include "Shader/GLShaderProgram.h"


namespace LLGL
{


GLRenderContext::GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext) :
    RenderContext   ( window, desc.videoMode ),
    desc_           ( desc                   )
{
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

/* ----- Configuration ----- */

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
        mask |= GL_COLOR_BUFFER_BIT;
    if ((flags & ClearBuffersFlags::Depth) != 0)
        mask |= GL_DEPTH_BUFFER_BIT;
    if ((flags & ClearBuffersFlags::Stencil) != 0)
        mask |= GL_STENCIL_BUFFER_BIT;

    glClear(mask);
}

void GLRenderContext::SetDrawMode(const DrawMode drawMode)
{
    renderState_.drawMode = GLTypeConversion::Map(drawMode);
}

/* ----- Hardware buffers ------ */

void GLRenderContext::BindVertexBuffer(VertexBuffer& vertexBuffer)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindVertexArray(vertexBufferGL.GetVaoID());
}

void GLRenderContext::UnbindVertexBuffer()
{
    GLStateManager::active->BindVertexArray(0);
}

void GLRenderContext::BindIndexBuffer(IndexBuffer& indexBuffer)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Store new index buffer data in global render state */
    renderState_.indexBufferDataType = GLTypeConversion::Map(indexBuffer.GetIndexFormat().GetDataType());
}

void GLRenderContext::UnbindIndexBuffer()
{
    GLStateManager::active->BindBuffer(GLBufferTarget::ELEMENT_ARRAY_BUFFER, 0);
}

void GLRenderContext::BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    GLStateManager::active->BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, index, constantBufferGL.hwBuffer.GetID());
}

void GLRenderContext::UnbindConstantBuffer(unsigned int index)
{
    //todo...
}

/* --- Drawing --- */

void GLRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLRenderContext::DrawIndexed(unsigned int numVertices)
{
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        nullptr
    );
}

void GLRenderContext::DrawIndexed(unsigned int numVertices, int indexOffset)
{
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        nullptr,
        indexOffset
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

void GLRenderContext::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances)
{
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        nullptr,
        static_cast<GLsizei>(numInstances)
    );
}

void GLRenderContext::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset)
{
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        nullptr,
        static_cast<GLsizei>(numInstances),
        indexOffset
    );
}

void GLRenderContext::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset, unsigned int instanceOffset)
{
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        nullptr,
        static_cast<GLsizei>(numInstances),
        indexOffset,
        instanceOffset
    );
}

/* ----- Shader ----- */

void GLRenderContext::BindShaderProgram(ShaderProgram& shaderProgram)
{
    /* Bind index buffer */
    auto& shaderProgramGL = LLGL_CAST(GLShaderProgram&, shaderProgram);
    GLStateManager::active->BindShaderProgram(shaderProgramGL.GetID());
}

void GLRenderContext::UnbindShaderProgram()
{
    GLStateManager::active->BindShaderProgram(0);
}

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
}

void GLRenderContext::InitRenderStates()
{
    /* Setup default render states to be uniform between render systems */
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // D3D10+ has this per default
    glFrontFace(GL_CW);                     // D3D10+ uses clock-wise vertex winding per default

    /*
    Set pixel storage to byte-alignment (default is word-alignment).
    This is required so that texture formats like RGB (which is not word-aligned) can be used.
    */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Initialize state manager */
    stateMngr_->Reset();

    #if 0 // !!! TESTING !!!
    glEnable(GL_DEPTH_TEST);
    #endif
}

void GLRenderContext::QueryGLVerion(GLint& major, GLint& minor)
{
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
}


} // /namespace LLGL



// ================================================================================
