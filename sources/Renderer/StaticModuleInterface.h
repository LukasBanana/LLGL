/*
 * StaticModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STATIC_MODULE_INTERFACE_H
#define LLGL_STATIC_MODULE_INTERFACE_H

#if LLGL_BUILD_STATIC_LIB


#include <LLGL/RenderSystemFlags.h>
#include <string>


namespace LLGL
{

class RenderSystem;

namespace StaticModules
{


// Returns the list of staticly compiled modules.
std::vector<std::string> GetStaticModules();

// Returns the renderer name of the specified module (module name "Direct3D11" may result in "Direct3D 11" for instance).
const char* GetRendererName(const std::string& moduleName);

// Returns the renderer ID of the specified module.
int GetRendererID(const std::string& moduleName);

// Allocates a new renderer system of the specified module. This is an owning raw pointer!
RenderSystem* AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc);


} // /namespace StaticModules

} // /namespace LLGL


#endif // /LLGL_BUILD_STATIC_LIB

#endif



// ================================================================================
