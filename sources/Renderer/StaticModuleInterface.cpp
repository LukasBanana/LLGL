/*
 * StaticModuleInterface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef LLGL_BUILD_STATIC_LIB


#include "StaticModuleInterface.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


class RenderSystem;

#define LLGL_DECLARE_STATIC_MODULE_INTERFACE(NAME)                                      \
    namespace Module##NAME                                                              \
    {                                                                                   \
        extern int GetRendererID();                                                     \
        extern const char* GetModuleName();                                             \
        extern const char* GetRendererName();                                           \
        extern RenderSystem* AllocRenderSystem(const LLGL::RenderSystemDescriptor*);    \
    }

#ifdef LLGL_BUILD_RENDERER_NULL
LLGL_DECLARE_STATIC_MODULE_INTERFACE(Null);
#endif

#ifdef LLGL_BUILD_RENDERER_OPENGL
LLGL_DECLARE_STATIC_MODULE_INTERFACE(OpenGL);
#endif

#ifdef LLGL_BUILD_RENDERER_OPENGLES3
LLGL_DECLARE_STATIC_MODULE_INTERFACE(OpenGLES3);
#endif

#ifdef LLGL_BUILD_RENDERER_WEBGL
LLGL_DECLARE_STATIC_MODULE_INTERFACE(WebGL);
#endif

#ifdef LLGL_BUILD_RENDERER_VULKAN
LLGL_DECLARE_STATIC_MODULE_INTERFACE(Vulkan);
#endif

#ifdef LLGL_BUILD_RENDERER_METAL
LLGL_DECLARE_STATIC_MODULE_INTERFACE(Metal);
#endif

#ifdef LLGL_BUILD_RENDERER_DIRECT3D11
LLGL_DECLARE_STATIC_MODULE_INTERFACE(Direct3D11);
#endif

#ifdef LLGL_BUILD_RENDERER_DIRECT3D12
LLGL_DECLARE_STATIC_MODULE_INTERFACE(Direct3D12);
#endif


namespace StaticModule
{


std::vector<std::string> GetStaticModules()
{
    return
    {
        #ifdef LLGL_BUILD_RENDERER_NULL
        ModuleNull::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_OPENGL
        ModuleOpenGL::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_OPENGLES3
        ModuleOpenGLES3::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_WEBGL
        ModuleWebGL::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_VULKAN
        ModuleVulkan::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_METAL
        ModuleMetal::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_DIRECT3D11
        ModuleDirect3D11::GetModuleName(),
        #endif
        #ifdef LLGL_BUILD_RENDERER_DIRECT3D12
        ModuleDirect3D12::GetModuleName(),
        #endif
    };
}

const char* GetRendererName(const std::string& moduleName)
{
    #define LLGL_GET_RENDERER_NAME(MODULE)          \
        if (moduleName == MODULE::GetModuleName())  \
            return MODULE::GetRendererName()

    #ifdef LLGL_BUILD_RENDERER_NULL
    LLGL_GET_RENDERER_NAME(ModuleNull);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGL
    LLGL_GET_RENDERER_NAME(ModuleOpenGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGLES3
    LLGL_GET_RENDERER_NAME(ModuleOpenGLES3);
    #endif

    #ifdef LLGL_BUILD_RENDERER_WEBGL
    LLGL_GET_RENDERER_NAME(ModuleWebGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_VULKAN
    LLGL_GET_RENDERER_NAME(ModuleVulkan);
    #endif

    #ifdef LLGL_BUILD_RENDERER_METAL
    LLGL_GET_RENDERER_NAME(ModuleMetal);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D11
    LLGL_GET_RENDERER_NAME(ModuleDirect3D11);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D12
    LLGL_GET_RENDERER_NAME(ModuleDirect3D12);
    #endif

    #undef LLGL_GET_RENDERER_NAME

    return nullptr;
}

int GetRendererID(const std::string& moduleName)
{
    #define LLGL_GET_RENDERER_ID(MODULE)            \
        if (moduleName == MODULE::GetModuleName())  \
            return MODULE::GetRendererID()

    #ifdef LLGL_BUILD_RENDERER_NULL
    LLGL_GET_RENDERER_ID(ModuleNull);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGL
    LLGL_GET_RENDERER_ID(ModuleOpenGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGLES3
    LLGL_GET_RENDERER_ID(ModuleOpenGLES3);
    #endif

    #ifdef LLGL_BUILD_RENDERER_WEBGL
    LLGL_GET_RENDERER_ID(ModuleWebGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_VULKAN
    LLGL_GET_RENDERER_ID(ModuleVulkan);
    #endif

    #ifdef LLGL_BUILD_RENDERER_METAL
    LLGL_GET_RENDERER_ID(ModuleMetal);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D11
    LLGL_GET_RENDERER_ID(ModuleDirect3D11);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D12
    LLGL_GET_RENDERER_ID(ModuleDirect3D12);
    #endif

    #undef LLGL_GET_RENDERER_ID

    return RendererID::Undefined;
}

RenderSystem* AllocRenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    #define LLGL_ALLOC_RENDER_SYSTEM(MODULE)                        \
        if (renderSystemDesc.moduleName == MODULE::GetModuleName()) \
            return MODULE::AllocRenderSystem(&renderSystemDesc)

    #ifdef LLGL_BUILD_RENDERER_NULL
    LLGL_ALLOC_RENDER_SYSTEM(ModuleNull);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGL
    LLGL_ALLOC_RENDER_SYSTEM(ModuleOpenGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_OPENGLES3
    LLGL_ALLOC_RENDER_SYSTEM(ModuleOpenGLES3);
    #endif

    #ifdef LLGL_BUILD_RENDERER_WEBGL
    LLGL_ALLOC_RENDER_SYSTEM(ModuleWebGL);
    #endif

    #ifdef LLGL_BUILD_RENDERER_VULKAN
    LLGL_ALLOC_RENDER_SYSTEM(ModuleVulkan);
    #endif

    #ifdef LLGL_BUILD_RENDERER_METAL
    LLGL_ALLOC_RENDER_SYSTEM(ModuleMetal);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D11
    LLGL_ALLOC_RENDER_SYSTEM(ModuleDirect3D11);
    #endif

    #ifdef LLGL_BUILD_RENDERER_DIRECT3D12
    LLGL_ALLOC_RENDER_SYSTEM(ModuleDirect3D12);
    #endif

    #undef LLGL_ALLOC_RENDER_SYSTEM

    return nullptr;
}


} // /namespace StaticModule

} // /namespace LLGL


#endif //LLGL_BUILD_STATIC_LIB



// ================================================================================
