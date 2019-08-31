/*
 * VKModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "VKRenderSystem.h"


namespace LLGL
{


namespace ModuleVulkan
{
    int GetRendererID()
    {
        return LLGL::RendererID::Vulkan;
    }

    const char* GetModuleName()
    {
        return "Vulkan";
    }

    const char* GetRendererName()
    {
        return "Vulkan";
    }

    RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor* renderSystemDesc)
    {
        return new VKRenderSystem(*renderSystemDesc);
    }
} // /namespace ModuleVulkan


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
    return LLGL::ModuleVulkan::GetRendererID();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return LLGL::ModuleVulkan::GetRendererName();
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
{
    auto desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
    return LLGL::ModuleVulkan::AllocRenderSystem(desc);
}

} // /extern "C"

#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
