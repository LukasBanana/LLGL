/*
 * DynamicModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

/**
\brief Allocates the render system and returns it as raw pointer.
\param[in] renderSystemDesc Specifies the descriptor for this render system. This must be re-interpret casted to RenderSystemDescriptor.
\param[in] renderSystemDescSize Specifies the size of the descriptor. This must be equal to <tt>sizeof(RenderSystemDescriptor)</tt>.
*/
LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc, int renderSystemDescSize);

/**
\brief Deletes the specified render system.
\remarks This function is optional and the default deleter will be used if this function is not present in a render system module.
*/
LLGL_EXPORT void LLGL_RenderSystem_Free(void* renderSystem);

#ifdef __cplusplus
} // /extern "C"
#endif


#endif



// ================================================================================
