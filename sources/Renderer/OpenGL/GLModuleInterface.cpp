/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"

namespace LLGL
{
    namespace ModuleOpengl
    {
        int RenderModuleID()
        {
            return LLGL::RendererID::OpenGL;
        }

        const char* RenderModuleName()
        {
            return "OpenGL";
        }

        void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
        {
            return new LLGL::GLRenderSystem(*desc);
        }
    }
}

extern "C"
{
#ifndef LLGL_BUILD_STATIC_LIB


    LLGL_EXPORT int LLGL_RenderSystem_RendererID(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleOpengl::RenderModuleID();
    }

    LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleOpengl::RenderModuleName();
    }

    LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
    {
        auto desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
        return LLGL::ModuleOpengl::RenderModuleCreate(desc);
    }

#endif

} // /extern "C"



// ================================================================================
