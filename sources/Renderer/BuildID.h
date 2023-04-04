/*
 * BuildID.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BUILD_ID_H
#define LLGL_BUILD_ID_H


// Increment this number when the interface of LLGL changes in any way
#define LLGL_BUILD_VERSION 7

#ifdef LLGL_DEBUG
#   if defined(_MSC_VER)
#       define LLGL_BUILD_ID (100000 + LLGL_BUILD_VERSION + _MSC_VER)
#   elif defined(__GNUC__)
#       define LLGL_BUILD_ID (200000 + LLGL_BUILD_VERSION + __GNUC__)
#   elif defined(__clang__)
#       define LLGL_BUILD_ID (300000 + LLGL_BUILD_VERSION + __clang__)
#   endif
#else
#   if defined(_MSC_VER)
#       define LLGL_BUILD_ID (110000 + LLGL_BUILD_VERSION + _MSC_VER)
#   elif defined(__GNUC__)
#       define LLGL_BUILD_ID (210000 + LLGL_BUILD_VERSION + __GNUC__)
#   elif defined(__clang__)
#       define LLGL_BUILD_ID (310000 + LLGL_BUILD_VERSION + __clang__)
#   endif
#endif


#endif



// ================================================================================
