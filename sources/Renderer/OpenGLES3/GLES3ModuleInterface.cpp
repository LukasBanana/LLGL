/*
 * GLES3ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
//#include "GLES3RenderSystem.h"
#include <LLGL/RenderSystemFlags.h>

namespace LLGL
{
    namespace detail_opengles3
    {
        int RenderModuleID()
        {
            return LLGL::RendererID::OpenGLES3;
        }

        const char* RenderModuleName()
        {
            return "OpenGL ES3";
        }

        void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
        {
            return nullptr;//TODO; could try angle to emulate gles3
        }
    }
}

extern "C"
{
#ifndef LLGL_BUILD_STATIC_LIB

    LLGL_EXPORT int LLGL_RenderSystem_BuildID()
    {
        return LLGL_BUILD_ID;
    }

    LLGL_EXPORT int LLGL_RenderSystem_RendererID(const void* /*renderSystemDesc*/)
    {
        return LLGL::detail_opengles3::RenderModuleID();
    }

    LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
    {
        return LLGL::detail_opengles3::RenderModuleName();
    }

    LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
    {
        return LLGL::detail_opengles3::RenderModuleCreate(nullptr);
    }

#endif

} // /extern "C"

// ================================================================================
