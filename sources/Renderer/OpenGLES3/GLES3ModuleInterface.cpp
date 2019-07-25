/*
 * GLES3ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
//#include "GLES3RenderSystem.h"


namespace LLGL
{


namespace ModuleOpenGLES3
{
    int GetRendererID()
    {
        return RendererID::OpenGLES3;
    }

    const char* GetModuleName()
    {
        return "OpenGLES3";
    }

    const char* GetRendererName()
    {
        return "OpenGL ES 3";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* /*renderSystemDesc*/)
    {
        return nullptr;//new GLES3RenderSystem();
    }
} // /namespace ModuleOpenGLES3


} // /namespace LLGL

#ifndef LLGL_BUILD_STATIC_LIB

extern "C"
{

LLGL_EXPORT int LLGL_RenderSystem_BuildID()
{
    return LLGL_BUILD_ID;
}

LLGL_EXPORT int LLGL_RenderSystem_RendererID()
{
    return LLGL::ModuleOpenGLES3::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleOpenGLES3::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
{
    return LLGL::ModuleOpenGLES3::AllocRenderSystem(nullptr);
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
