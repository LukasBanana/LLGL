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
    CreateContext(sharedRenderContext);
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


/*
 * ======= Private: =======
 */

void GLRenderContext::QueryGLVerion(GLint& major, GLint& minor)
{
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
}


} // /namespace LLGL



// ================================================================================
