/*
 * StaticModuleInterface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_BUILD_STATIC_LIB


#include "StaticModuleInterface.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/Strings.h>
#include "../Core/Assertion.h"
#include "../Core/CompilerExtensions.h"
#include <algorithm>


namespace LLGL
{


class RenderSystem;

#define LLGL_DECLARE_STATIC_MODULE(NAME) \
    extern LLGL::StaticModules::RegisterStaticModuleWrapper g_StaticModule_ ## NAME

#define LLGL_STATIC_MODULE_STUB(NAME) \
    g_StaticModule_ ## NAME .Stub()

#if LLGL_BUILD_RENDERER_NULL
LLGL_DECLARE_STATIC_MODULE(Null);
#endif

#if LLGL_BUILD_RENDERER_OPENGL
LLGL_DECLARE_STATIC_MODULE(OpenGL);
#endif

#if LLGL_BUILD_RENDERER_OPENGLES3
LLGL_DECLARE_STATIC_MODULE(OpenGLES3);
#endif

#if LLGL_BUILD_RENDERER_WEBGL
LLGL_DECLARE_STATIC_MODULE(WebGL);
#endif

#if LLGL_BUILD_RENDERER_VULKAN
LLGL_DECLARE_STATIC_MODULE(Vulkan);
#endif

#if LLGL_BUILD_RENDERER_METAL
LLGL_DECLARE_STATIC_MODULE(Metal);
#endif

#if LLGL_BUILD_RENDERER_DIRECT3D11
LLGL_DECLARE_STATIC_MODULE(Direct3D11);
#endif

#if LLGL_BUILD_RENDERER_DIRECT3D12
LLGL_DECLARE_STATIC_MODULE(Direct3D12);
#endif

/*
The sole purpose of this function is to force the inclusion of all those backends by the linker when building as static library.
This, unfortunately, creats a cyclic dependency between the core library and its backends.
The alternative is to let each project that uses LLGL call a stub function of the backend that it wants to include at link time.
*/
static void StaticModuleStubs()
{
    #if LLGL_BUILD_RENDERER_NULL
    LLGL_STATIC_MODULE_STUB(Null);
    #endif

    #if LLGL_BUILD_RENDERER_OPENGL
    LLGL_STATIC_MODULE_STUB(OpenGL);
    #endif

    #if LLGL_BUILD_RENDERER_OPENGLES3
    LLGL_STATIC_MODULE_STUB(OpenGLES3);
    #endif

    #if LLGL_BUILD_RENDERER_WEBGL
    LLGL_STATIC_MODULE_STUB(WebGL);
    #endif

    #if LLGL_BUILD_RENDERER_VULKAN
    LLGL_STATIC_MODULE_STUB(Vulkan);
    #endif

    #if LLGL_BUILD_RENDERER_METAL
    LLGL_STATIC_MODULE_STUB(Metal);
    #endif

    #if LLGL_BUILD_RENDERER_DIRECT3D11
    LLGL_STATIC_MODULE_STUB(Direct3D11);
    #endif

    #if LLGL_BUILD_RENDERER_DIRECT3D12
    LLGL_STATIC_MODULE_STUB(Direct3D12);
    #endif
}

namespace StaticModules
{


// Wrapper function to ensure the static container is initialized by the time we need it,
// since this gets called by initializer expressions of other global static variables, for which there is no guarantee in which order they are initialized.
static std::vector<StaticModuleRecord>& GetStaticModuleList()
{
    static std::vector<StaticModuleRecord> staticModuleRecords;
    return staticModuleRecords;
}

static const StaticModuleRecord* FindStaticModule(const char* name)
{
    const std::vector<StaticModuleRecord>& moduleRecords = GetStaticModuleList();
    auto it = std::find_if(
        moduleRecords.begin(),
        moduleRecords.end(),
        [name](const StaticModuleRecord& entry)
        {
            return (entry.moduleName == name);
        }
    );
    return (it != moduleRecords.end() ? &(*it) : nullptr);
}

void RegisterStaticModule(StaticModuleRecord&& moduleRecord)
{
    LLGL_ASSERT_PTR(moduleRecord.funcGetRendererID);
    LLGL_ASSERT_PTR(moduleRecord.funcGetRendererName);
    LLGL_ASSERT_PTR(moduleRecord.funcAllocRenderSystem);

    /* Insert new module record via insertion sort by its priority */
    std::vector<StaticModuleRecord>& allModuleRecords = GetStaticModuleList();
    auto it = std::find_if(
        allModuleRecords.begin(),
        allModuleRecords.end(),
        [&moduleRecord](const StaticModuleRecord& entry) -> bool
        {
            return (entry.priority < moduleRecord.priority);
        }
    );
    if (it != allModuleRecords.end())
        allModuleRecords.insert(it, std::move(moduleRecord));
    else
        allModuleRecords.push_back(std::move(moduleRecord));
}

RegisterStaticModuleWrapper::RegisterStaticModuleWrapper(StaticModuleRecord&& moduleRecord)
{
    RegisterStaticModule(std::forward<StaticModuleRecord>(moduleRecord));
}

std::vector<std::string> GetStaticModules()
{
    std::vector<std::string> staticModuleNames;
    staticModuleNames.reserve(GetStaticModuleList().size());

    for (const StaticModuleRecord& moduleRecord : GetStaticModuleList())
        staticModuleNames.push_back(moduleRecord.moduleName);

    return staticModuleNames;
}

const char* GetRendererName(const StringLiteral& moduleName)
{
    if (const StaticModuleRecord* moduleRecord = FindStaticModule(moduleName.c_str()))
        return moduleRecord->funcGetRendererName();
    else
        return nullptr;
}

int GetRendererID(const StringLiteral& moduleName)
{
    if (const StaticModuleRecord* moduleRecord = FindStaticModule(moduleName.c_str()))
        return moduleRecord->funcGetRendererID();
    else
        return RendererID::Undefined;
}

RenderSystem* AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    StaticModuleStubs();
    if (const StaticModuleRecord* moduleRecord = FindStaticModule(renderSystemDesc.moduleName.c_str()))
        return moduleRecord->funcAllocRenderSystem(&renderSystemDesc);
    else
        return nullptr;
}

LLGL_BEGIN_NO_OPTIMIZE
void RegisterStaticModuleWrapper::Stub()
{
    // do nothing - this is to ensure the global variables of this wrapper are included by the linker
}
LLGL_END_NO_OPTIMIZE


} // /namespace StaticModules

} // /namespace LLGL


#endif //LLGL_BUILD_STATIC_LIB



// ================================================================================
