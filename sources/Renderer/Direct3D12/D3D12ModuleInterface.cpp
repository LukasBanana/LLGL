/*
 * D3D12ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D12RenderSystem.h"


extern "C"
{

LLGL_EXPORT int LLGL_RenderSystem_BuildID()
{
    return LLGL_BUILD_ID;
}

LLGL_EXPORT int LLGL_RenderSystem_RendererID()
{
    return LLGL::RendererID::Direct3D12;
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return "Direct3D 12";
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
{
    return new LLGL::D3D12RenderSystem();
}

} // /extern "C"



// ================================================================================
