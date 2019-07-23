/*
 * MTModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "MTRenderSystem.h"


namespace LLGL
{
    namespace ModuleMetal
    {
        int RenderModuleID()
        {
            return LLGL::RendererID::Metal;
        }

        const char* RenderModuleName()
        {
            return "Metal";
        }

        void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
        {
            return new LLGL::MTRenderSystem();
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
        return LLGL::ModuleMetal::RenderModuleID();
    }

    LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleMetal::RenderModuleName();
    }

    LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleMetal::RenderModuleCreate(nullptr);
    }

#endif

} // /extern "C"



// ================================================================================
