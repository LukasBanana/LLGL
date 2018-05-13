/*
 * VKExtensions.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKExtensions.h"


namespace LLGL
{

    
/* Platform specific VL extensions */

#if defined(LLGL_OS_WIN32)

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;

#elif defined(LLGL_OS_LINUX)

//???

#endif


} // /namespace LLGL



// ================================================================================
