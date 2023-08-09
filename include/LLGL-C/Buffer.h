/*
 * Buffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_BUFFER_H
#define LLGL_C99_BUFFER_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT long llglGetBufferBindFlags(LLGLBuffer buffer);
LLGL_C_EXPORT void llglGetBufferDesc(LLGLBuffer buffer, LLGLBufferDescriptor* outDesc);


#endif



// ================================================================================
