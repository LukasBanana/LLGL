/*
 * VKExtensions.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_EXTENSIONS_H
#define LLGL_VK_EXTENSIONS_H


#include "../Vulkan.h"


namespace LLGL
{


/* Platform specific VL extensions */

#if defined(LLGL_OS_WIN32)

extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

#elif defined(LLGL_OS_LINUX)

//???

#endif


} // /namespace LLGL


#endif



// ================================================================================
