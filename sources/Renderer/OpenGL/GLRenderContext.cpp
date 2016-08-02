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

std::string GLRenderContext::GetVersion() const
{
    return ""; //todo...
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
