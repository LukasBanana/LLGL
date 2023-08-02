/*
 * Export.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C_EXPORT_H
#define LLGL_C_EXPORT_H


#include <LLGL/Export.h>


#ifdef __cplusplus
#   define LLGL_C_EXPORT extern "C" LLGL_EXPORT
#else
#   define LLGL_C_EXPORT LLGL_EXPORT
#endif


#endif



// ================================================================================
