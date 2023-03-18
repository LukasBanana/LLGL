/*
 * IOSDebug.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IOS_DEBUG_H
#define LLGL_IOS_DEBUG_H


#include <signal.h>

#ifdef SIGTRAP
#   define LLGL_DEBUG_BREAK() raise(SIGTRAP)
#else
#   define LLGL_DEBUG_BREAK()
#endif


#endif



// ================================================================================
