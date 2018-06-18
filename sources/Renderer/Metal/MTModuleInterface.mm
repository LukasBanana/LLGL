/*
 * MTModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "MTRenderSystem.h"


extern "C"
{

LLGL_EXPORT int LLGL_RenderSystem_BuildID()
{
    return LLGL_BUILD_ID;
}

LLGL_EXPORT int LLGL_RenderSystem_RendererID()
{
    return LLGL::RendererID::Metal;
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return "Metal";
}

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
{
    return new LLGL::MTRenderSystem();
}

}



// ================================================================================
