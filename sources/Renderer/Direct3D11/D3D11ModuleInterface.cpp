/*
 * D3D11ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D11RenderSystem.h"


extern "C"
{

LLGL_EXPORT void* LLGL_RenderSystem_Alloc()
{
    return new LLGL::D3D11RenderSystem();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return "Direct3D 11";
}

}



// ================================================================================
