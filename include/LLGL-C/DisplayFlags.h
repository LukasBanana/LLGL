/*
 * DisplayFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_DISPLAY_FLAGS_H
#define LLGL_C99_DISPLAY_FLAGS_H


#include <LLGL-C/Types.h>
#include <stdint.h>


/* ----- Structures ----- */

typedef struct LLGLDisplayModeDescriptor
{
    LLGLExtent2D    resolution;
    uint32_t        refreshRate;
}
LLGLDisplayModeDescriptor;


#endif



// ================================================================================
