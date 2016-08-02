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
    desc_   ( desc   ),
    window_ ( window )
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


} // /namespace LLGL



// ================================================================================
