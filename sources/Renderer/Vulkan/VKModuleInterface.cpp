/*
 * VKModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "VKRenderSystem.h"


namespace LLGL
{
	namespace detail_vulkan
	{
		int RenderModuleID()
		{
		    return LLGL::RendererID::Vulkan;
		}

		const char* RenderModuleName()
		{
		    return "Vulkan";
		}

		void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
		{	
    		return new LLGL::VKRenderSystem(*desc);
		}
	}
}

extern "C"
{
	
#ifndef LLGL_BUILD_STATIC_LIB

	LLGL_EXPORT int LLGL_RenderSystem_BuildID()
	{
	    return LLGL_BUILD_ID;
	}

	LLGL_EXPORT int LLGL_RenderSystem_RendererID(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_vulkan::RenderModuleID();
	}

	LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_vulkan::RenderModuleName();
	}

	LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* renderSystemDesc)
	{
		auto desc = reinterpret_cast<const LLGL::RenderSystemDescriptor*>(renderSystemDesc);
	    return LLGL::detail_vulkan::RenderModuleCreate(desc);
	}

#endif

} // /extern "C"




// ================================================================================
