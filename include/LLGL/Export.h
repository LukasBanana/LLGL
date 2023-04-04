/*
 * Export.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXPORT_H
#define LLGL_EXPORT_H


#if defined _MSC_VER && !defined LLGL_BUILD_STATIC_LIB
#   define LLGL_EXPORT __declspec(dllexport)
#else
#   define LLGL_EXPORT
#endif


#endif



// ================================================================================
