/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ModuleInterface.h"
#include <LLGL/RenderSystemFlags.h>

#ifdef LLGL_BUILD_STATIC_LIB

namespace LLGL
{
    
#ifdef LLGL_HAS_MODULE_DX11
    namespace ModuleD3D11
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_DX12
    namespace ModuleD3D12
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_OPENGL
    namespace ModuleOpengl
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_OPENGLES3
    namespace ModuleOpenglES3
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_VULKAN
    namespace ModuleVulkan
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif
    
#ifdef LLGL_HAS_MODULE_METAL
    namespace ModuleMetal
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

    void GetStaticModules(std::vector<std::string> & out)
    {
#ifdef LLGL_HAS_MODULE_DX11
        out.push_back(LLGL::ModuleD3D11::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_DX12
        out.push_back(LLGL::ModuleD3D12::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        out.push_back(LLGL::ModuleOpengl::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        out.push_back(LLGL::ModuleOpenglES3::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        out.push_back(LLGL::ModuleVulkan::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_METAL
        out.push_back(LLGL::ModuleMetal::RenderModuleName());
#endif
    }

}

//when static linking we load all rederers from here
extern "C"
{
    LLGL_EXPORT int LLGL_RenderSystem_BuildID()
    {
        return LLGL_BUILD_ID;
    }

    LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* renderSystemDesc)
    {
        const auto* desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
        //same result given by GetStaticModules
        return desc->moduleName.c_str();
    }

    LLGL_EXPORT int LLGL_RenderSystem_RendererID(const void* renderSystemDesc)
    {
        const auto* desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
#ifdef LLGL_HAS_MODULE_DX11
        if (desc->moduleName == LLGL::ModuleD3D11::RenderModuleName()) return LLGL::ModuleD3D11::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_DX12
        if (desc->moduleName == LLGL::ModuleD3D12::RenderModuleName()) return LLGL::ModuleD3D12::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        if (desc->moduleName == LLGL::ModuleOpengl::RenderModuleName()) return LLGL::ModuleOpengl::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        if (desc->moduleName == LLGL::ModuleOpenglES3::RenderModuleName()) return LLGL::ModuleOpenglES3::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        if (desc->moduleName == LLGL::ModuleVulkan::RenderModuleName()) return LLGL::ModuleVulkan::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_METAL
        if (desc->moduleName == LLGL::ModuleMetal::RenderModuleName()) return LLGL::ModuleMetal::RenderModuleID();
#endif
        return -1;
    }

    LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
    {
        const auto* desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
#ifdef LLGL_HAS_MODULE_DX11
        if (desc->moduleName == LLGL::ModuleD3D11::RenderModuleName()) return LLGL::ModuleD3D11::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_DX12
        if (desc->moduleName == LLGL::ModuleD3D12::RenderModuleName()) return LLGL::ModuleD3D12::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        if (desc->moduleName == LLGL::ModuleOpengl::RenderModuleName()) return LLGL::ModuleOpengl::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        if (desc->moduleName == LLGL::ModuleOpenglES3::RenderModuleName()) return LLGL::ModuleOpenglES3::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        if (desc->moduleName == LLGL::ModuleVulkan::RenderModuleName()) return LLGL::ModuleVulkan::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_METAL
        if (desc->moduleName == LLGL::ModuleMetal::RenderModuleName()) return LLGL::ModuleMetal::RenderModuleCreate(desc);
#endif
        return nullptr;
    }

} // /extern "C"

#endif //LLGL_BUILD_STATIC_LIB

// ================================================================================
