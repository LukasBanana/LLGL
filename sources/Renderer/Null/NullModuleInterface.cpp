/*
 * NullModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ModuleInterface.h"
#include "NullRenderSystem.h"


namespace LLGL
{


namespace ModuleNull
{
    int GetRendererID()
    {
        return RendererID::Null;
    }

    const char* GetModuleName()
    {
        return "Null";
    }

    const char* GetRendererName()
    {
        return "Null";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* renderSystemDesc)
    {
        return new NullRenderSystem(*renderSystemDesc);
    }
} // /namespace ModuleNull


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
    return LLGL::ModuleNull::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleNull::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc, int renderSystemDescSize)
{
    if (renderSystemDesc != nullptr && static_cast<std::size_t>(renderSystemDescSize) == sizeof(LLGL::RenderSystemDescriptor))
    {
        auto desc = static_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
        return LLGL::ModuleNull::AllocRenderSystem(desc);
    }
    return nullptr;
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
