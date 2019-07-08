/*
 * D3D11ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D11RenderSystem.h"


namespace LLGL
{
    namespace ModuleD3D11
    {
        LLGL_EXPORT int RenderModuleID()
        {
            return LLGL::RendererID::Direct3D11;
        }

        LLGL_EXPORT const char* RenderModuleName()
        {
            return "Direct3D 11";
        }

        LLGL_EXPORT void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
        {
            return new LLGL::D3D11RenderSystem();
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
        return LLGL::ModuleD3D11::RenderModuleID();
    }

    LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleD3D11::RenderModuleName();
    }

    LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
    {
        return LLGL::ModuleD3D11::RenderModuleCreate(nullptr);
    }

#endif

} // /extern "C"



// ================================================================================
