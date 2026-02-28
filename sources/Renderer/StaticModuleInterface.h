/*
 * StaticModuleInterface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STATIC_MODULE_INTERFACE_H
#define LLGL_STATIC_MODULE_INTERFACE_H


#include <LLGL/RenderSystemFlags.h>
#include <string>
#include <functional>


/*
Helper macro to register a static renderer module. The module needs to implement the following functions:
namespace Module<NAME> {
  int GetRendererID();
  const char* GetRendererName();
  LLGL::RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor*);
}
*/
#define LLGL_IMPLEMENT_RENDERER_MODULE(NAME, PRIORITY)                          \
    LLGL::StaticModules::RegisterStaticModuleWrapper g_StaticModule_ ## NAME    \
    {                                                                           \
        LLGL::StaticModules::StaticModuleRecord                                 \
        {                                                                       \
            #NAME,                                                              \
            Module ## NAME :: GetRendererID,                                    \
            Module ## NAME :: GetRendererName,                                  \
            Module ## NAME :: AllocRenderSystem,                                \
            (PRIORITY),                                                         \
        }                                                                       \
    }

namespace LLGL
{

class RenderSystem;

namespace StaticModules
{


// Helper struct to register static renderer modules.
struct StaticModuleRecord
{
    std::string                                                         moduleName;
    std::function<int()>                                                funcGetRendererID;
    std::function<const char*()>                                        funcGetRendererName;
    std::function<RenderSystem*(const LLGL::RenderSystemDescriptor*)>   funcAllocRenderSystem;
    int                                                                 priority;
};

// Wrapper structure to register a static renderer module. Don't use this directly; Instead, use LLGL_IMPLEMENT_RENDERER_MODULE() macro.
struct RegisterStaticModuleWrapper
{
    RegisterStaticModuleWrapper(StaticModuleRecord&& moduleRecord);
    void Stub();
};

// Registers a new static module. Static modules cannot be unregistered. This is used to avoid cyclic dependencies between LLGL's core library and its backends.
void RegisterStaticModule(StaticModuleRecord&& moduleRecord);

// Returns the list of staticly compiled modules.
std::vector<std::string> GetStaticModules();

// Returns the renderer name of the specified module (module name "Direct3D11" may result in "Direct3D 11" for instance).
const char* GetRendererName(const StringLiteral& moduleName);

// Returns the renderer ID of the specified module.
int GetRendererID(const StringLiteral& moduleName);

// Allocates a new renderer system of the specified module. This is an owning raw pointer!
RenderSystem* AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc);


} // /namespace StaticModules

} // /namespace LLGL


#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
