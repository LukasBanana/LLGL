/*
 * D3D12ModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "D3D12RenderSystem.h"


namespace LLGL
{
	namespace detail_dx12
	{
		int RenderModuleID()
		{
		    return LLGL::RendererID::Direct3D12;
		}

		const char* RenderModuleName()
		{
		    return "Direct3D 12";
		}

		void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
		{
		    return new LLGL::D3D12RenderSystem();
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
	    return LLGL::detail_dx12::RenderModuleID();
	}

	LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_dx12::RenderModuleName();
	}

	LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_dx12::RenderModuleCreate(nullptr);
	}

#endif

} // /extern "C"



// ================================================================================
