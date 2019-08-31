/*
 * StaticModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STATIC_MODULE_INTERFACE_H
#define LLGL_STATIC_MODULE_INTERFACE_H

#ifdef LLGL_BUILD_STATIC_LIB


#include <LLGL/RenderSystemFlags.h>
#include <string>


namespace LLGL
{

class RenderSystem;

namespace StaticModule
{


// Returns the list of staticly compiled modules.
std::vector<std::string> GetStaticModules();

// Returns the renderer name of the specified module (module name "Direct3D11" may result in "Direct3D 11" for instance).
const char* GetRendererName(const std::string& moduleName);

// Returns the renderer ID of the specified module.
int GetRendererID(const std::string& moduleName);

// Allocates a new renderer system of the specified module. This is an owning raw pointer!
RenderSystem* AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc);


} // /namespace StaticModule

} // /namespace LLGL


#endif // /LLGL_BUILD_STATIC_LIB

#endif



// ================================================================================
