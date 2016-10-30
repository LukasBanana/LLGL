/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"


extern "C"
{

LLGL_EXPORT int LLGL_RenderSystem_BuildID()
{
    return LLGL_BUILD_ID;
}

LLGL_EXPORT int LLGL_RenderSystem_RendererID()
{
    #ifdef LLGL_GL_OPENGLES
    return LLGL::RendererID::OpenGLES2;
    #else
    return LLGL::RendererID::OpenGL;
    #endif
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    #ifdef LLGL_GL_OPENGLES
    return "OpenGL ES";
    #else
    return "OpenGL";
    #endif
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc()
{
    return new LLGL::GLRenderSystem();
}

}



// ================================================================================
