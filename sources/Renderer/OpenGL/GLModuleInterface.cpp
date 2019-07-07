/*
 * GLModuleInterface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../ModuleInterface.h"
#include "GLRenderSystem.h"

namespace LLGL
{
	namespace detail_opengl
	{
		int RenderModuleID()
		{
		    return LLGL::RendererID::OpenGL;
		}

		const char* RenderModuleName()
		{
		    return "OpenGL";
		}

		void* RenderModuleCreate(const LLGL::RenderSystemDescriptor* desc)
		{
		    return new LLGL::GLRenderSystem();
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
	    return LLGL::detail_opengl::RenderModuleID();
	}

	LLGL_EXPORT const char* LLGL_RenderSystem_Name(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_opengl::RenderModuleName();
	}

	LLGL_EXPORT void* LLGL_RenderSystem_Alloc(const void* /*renderSystemDesc*/)
	{
	    return LLGL::detail_opengl::RenderModuleCreate(nullptr);
	}

#endif

} // /extern "C"



// ================================================================================
