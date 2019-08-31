/*
 * DynamicModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DYNAMIC_MODULE_INTERFACE_H
#define LLGL_DYNAMIC_MODULE_INTERFACE_H


#include "BuildID.h"
#include <LLGL/Export.h>


#ifdef __cplusplus
extern "C"
{
#endif

/*
Returns the build ID number of the render system.
This depends on the type and version of the used compiler, the debug/release mode, and an internal build version.
The returned value must be equal to the value of the LLGL_BUILD_ID macro.
Otherwise the render system might not be loaded correctly.
*/
LLGL_EXPORT int LLGL_RenderSystem_BuildID();

// Returns the renderer ID (see LLGL::RendererID).
LLGL_EXPORT int LLGL_RenderSystem_RendererID();

// Returns the name of this render system module (e.g. "OpenGL" or "Direct3D 11").
LLGL_EXPORT const char* LLGL_RenderSystem_Name();

// Returns a raw pointer to the allocated render system (allocated with "new" keyword)
LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc);

#ifdef __cplusplus
} // /extern "C"
#endif


#endif



// ================================================================================
