/*
 * ImageFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_IMAGE_FLAGS_H
#define LLGL_C99_IMAGE_FLAGS_H


#include <LLGL-C/Format.h>
#include <stdint.h>
#include <stddef.h>


/* ----- Structures ----- */

typedef struct LLGLSrcImageDescriptor
{
    LLGLImageFormat format;
    LLGLDataType    dataType;
    const void*     data;
    size_t          dataSize;
}
LLGLSrcImageDescriptor;

typedef struct LLGLDstImageDescriptor
{
    LLGLImageFormat format;
    LLGLDataType    dataType;
    void*           data;
    size_t          dataSize;
}
LLGLDstImageDescriptor;


#endif



// ================================================================================
