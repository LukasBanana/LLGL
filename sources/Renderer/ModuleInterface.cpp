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
	namespace detail_dx11
	{
		extern int RenderModuleID();
		extern const char* RenderModuleName();
		extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
	}
#endif

#ifdef LLGL_HAS_MODULE_DX12
    namespace detail_dx12
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_OPENGL
    namespace detail_opengl
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_OPENGLES3
    namespace detail_opengles3
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

#ifdef LLGL_HAS_MODULE_VULKAN
    namespace detail_vulkan
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif
    
#ifdef LLGL_HAS_MODULE_METAL
    namespace detail_metal
    {
        extern int RenderModuleID();
        extern const char* RenderModuleName();
        extern void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc);
    }
#endif

    void GetStaticModules(std::vector<std::string> & out)
    {
#ifdef LLGL_HAS_MODULE_DX11
        out.push_back(LLGL::detail_dx11::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_DX12
        out.push_back(LLGL::detail_dx12::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        out.push_back(LLGL::detail_opengl::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        out.push_back(LLGL::detail_opengles3::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        out.push_back(LLGL::detail_vulkan::RenderModuleName());
#endif
#ifdef LLGL_HAS_MODULE_METAL
        out.push_back(LLGL::detail_metal::RenderModuleName());
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
        if (desc->moduleName == LLGL::detail_dx11::RenderModuleName()) return LLGL::detail_dx11::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_DX12
        if (desc->moduleName == LLGL::detail_dx12::RenderModuleName()) return LLGL::detail_dx12::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        if (desc->moduleName == LLGL::detail_opengl::RenderModuleName()) return LLGL::detail_opengl::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        if (desc->moduleName == LLGL::detail_opengles3::RenderModuleName()) return LLGL::detail_opengles3::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        if (desc->moduleName == LLGL::detail_vulkan::RenderModuleName()) return LLGL::detail_vulkan::RenderModuleID();
#endif
#ifdef LLGL_HAS_MODULE_METAL
        if (desc->moduleName == LLGL::detail_metal::RenderModuleName()) return LLGL::detail_metal::RenderModuleID();
#endif
        return -1;
	}

	LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
	{
	    const auto* desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
#ifdef LLGL_HAS_MODULE_DX11
        if (desc->moduleName == LLGL::detail_dx11::RenderModuleName()) return LLGL::detail_dx11::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_DX12
        if (desc->moduleName == LLGL::detail_dx12::RenderModuleName()) return LLGL::detail_dx12::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_OPENGL
        if (desc->moduleName == LLGL::detail_opengl::RenderModuleName()) return LLGL::detail_opengl::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_OPENGLES3
        if (desc->moduleName == LLGL::detail_opengles3::RenderModuleName()) return LLGL::detail_opengles3::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_VULKAN
        if (desc->moduleName == LLGL::detail_vulkan::RenderModuleName()) return LLGL::detail_vulkan::RenderModuleCreate(desc);
#endif
#ifdef LLGL_HAS_MODULE_METAL
        if (desc->moduleName == LLGL::detail_metal::RenderModuleName()) return LLGL::detail_metal::RenderModuleCreate(desc);
#endif
        return nullptr;
	}

} // /extern "C"

#endif //LLGL_BUILD_STATIC_LIB

// ================================================================================
