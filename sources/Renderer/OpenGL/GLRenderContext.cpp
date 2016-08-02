/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"


namespace LLGL
{


GLRenderContext::GLRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext) :
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

/* ----- Rendering ----- */

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
}

void GLRenderContext::QueryGLVerion(GLint& major, GLint& minor)
{
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
}


} // /namespace LLGL



// ================================================================================
