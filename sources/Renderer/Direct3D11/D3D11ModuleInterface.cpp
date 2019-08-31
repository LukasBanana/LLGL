/*
 * D3D11ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D11RenderSystem.h"


namespace LLGL
{


namespace ModuleDirect3D11
{
    int GetRendererID()
    {
        return RendererID::Direct3D11;
    }

    const char* GetModuleName()
    {
        return "Direct3D11";
    }

    const char* GetRendererName()
    {
        return "Direct3D 11";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* /*renderSystemDesc*/)
    {
        return new D3D11RenderSystem();
    }
} // /namespace ModuleDirect3D11


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
    return LLGL::ModuleDirect3D11::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleDirect3D11::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
{
    return LLGL::ModuleDirect3D11::AllocRenderSystem(nullptr);
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
