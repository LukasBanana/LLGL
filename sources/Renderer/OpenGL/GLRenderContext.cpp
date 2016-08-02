/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"


namespace LLGL
{


GLRenderContext::GLRenderContext(const RenderContextDescriptor& desc, Window& window, GLRenderContext* sharedRenderContext) :
    desc_( desc )
{
    CreateContext(window, sharedRenderContext);
}

GLRenderContext::~GLRenderContext()
{
    DeleteContext();
}

std::string GLRenderContext::GetVersion() const
{
    return ""; //todo...
}


} // /namespace LLGL



// ================================================================================
