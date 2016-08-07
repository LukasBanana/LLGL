/*
 * ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_MODULE_INTERFACE_H__
#define __LLGL_MODULE_INTERFACE_H__


#include <LLGL/Export.h>


extern "C"
{

//! Returns a raw pointer to the allocated render system (allocated with "new" keyword)
LLGL_EXPORT void* LLGL_RenderSystem_Alloc(void* profiler);

//! Returns the name of this audio system module.
LLGL_EXPORT const char* LLGL_RenderSystem_Name();

}


#endif



// ================================================================================
