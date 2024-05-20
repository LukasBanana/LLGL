/*
 * UWPDebug.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_UWP_DEBUG_H
#define LLGL_UWP_DEBUG_H


#ifdef _DEBUG
#   define LLGL_DEBUG_BREAK() __debugbreak()
#else
#   define LLGL_DEBUG_BREAK()
#endif


#endif



// ================================================================================
