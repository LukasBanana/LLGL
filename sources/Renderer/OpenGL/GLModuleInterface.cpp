/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"
#include "GLRenderSystemProfiler.h"


extern "C"
{

LLGL_EXPORT void* LLGL_RenderSystem_Alloc(void* profiler)
{
    if (profiler)
        return new LLGL::GLRenderSystemProfiler(*reinterpret_cast<LLGL::RenderingProfiler*>(profiler));
    else
        return new LLGL::GLRenderSystem();
}

LLGL_EXPORT const char* LLGL_RenderSystem_Name()
{
    return "OpenGL";
}

}



// ================================================================================
