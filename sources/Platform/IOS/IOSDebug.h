/*
 * IOSDebug.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
