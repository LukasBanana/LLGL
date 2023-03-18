/*
 * Win32Debug.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
