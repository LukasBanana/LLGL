/*
 * D3D9ModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ModuleInterface.h"
#include "D3D9RenderSystem.h"


namespace LLGL
{


namespace ModuleDirect3D9
{
    int GetRendererID()
    {
        return RendererID::Direct3D9;
    }

    const char* GetRendererName()
    {
        return "Direct3D 9";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* renderSystemDesc)
    {
        return new D3D9RenderSystem{ *renderSystemDesc };
    }
} // /namespace ModuleDirect3D9

LLGL_IMPLEMENT_RENDERER_MODULE(Direct3D9, 1009);


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
    return LLGL::ModuleDirect3D9::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleDirect3D9::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc, int renderSystemDescSize)
{
    if (renderSystemDesc != nullptr && static_cast<std::size_t>(renderSystemDescSize) == sizeof(LLGL::RenderSystemDescriptor))
    {
        auto desc = static_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
        return LLGL::ModuleDirect3D9::AllocRenderSystem(desc);
    }
    return nullptr;
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
