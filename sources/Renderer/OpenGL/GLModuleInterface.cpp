/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"


namespace LLGL
{


namespace ModuleOpenGL
{
    int GetRendererID()
    {
        return RendererID::OpenGL;
    }

    const char* GetModuleName()
    {
        return "OpenGL";
    }

    const char* GetRendererName()
    {
        return "OpenGL";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* renderSystemDesc)
    {
        return new GLRenderSystem(*renderSystemDesc);
    }
} // /namespace ModuleOpenGL


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
    return LLGL::ModuleOpenGL::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleOpenGL::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
{
    auto desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
    return LLGL::ModuleOpenGL::AllocRenderSystem(desc);
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
