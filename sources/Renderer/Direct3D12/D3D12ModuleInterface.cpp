/*
 * D3D12ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D12RenderSystem.h"


namespace LLGL
{


namespace ModuleDirect3D12
{
    int GetRendererID()
    {
        return RendererID::Direct3D12;
    }

    const char* GetModuleName()
    {
        return "Direct3D12";
    }

    const char* GetRendererName()
    {
        return "Direct3D 12";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* /*renderSystemDesc*/)
    {
        return new D3D12RenderSystem();
    }
} // /namespace ModuleDirect3D12


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
    return LLGL::ModuleDirect3D12::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleDirect3D12::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
{
    return LLGL::ModuleDirect3D12::AllocRenderSystem(nullptr);
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
