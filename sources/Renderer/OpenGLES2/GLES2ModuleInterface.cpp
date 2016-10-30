/*
 * GLES2ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLES2RenderSystem.h"


extern "C"
{

LLGL_EXPORT int LLGL_RenderSystem_BuildID()
{
    return LLGL_BUILD_ID;
}

LLGL_EXPORT int LLGL_RenderSystem_RendererID()
{
    return LLGL::RendererID::OpenGLES2;
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return "OpenGL ES";
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc()
{
    return new LLGL::GLES2RenderSystem();
}

}



// ================================================================================
