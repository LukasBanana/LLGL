/*
 * Win32Debug.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_DEBUG_H
#define LLGL_WIN32_DEBUG_H


#ifdef _DEBUG
#   if defined _MSC_VER
#       define LLGL_DEBUG_BREAK() __debugbreak()
#   elif defined __GNUC__
#       include <signal.h>
#       define LLGL_DEBUG_BREAK() raise(SIGTRAP)
#   else
#       define LLGL_DEBUG_BREAK()
#   endif
#else
#   define LLGL_DEBUG_BREAK()
#endif


#endif



// ================================================================================
