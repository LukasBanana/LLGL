/*
 * BufferFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_BUFFER_FLAG_H
#define LLGL_C99_BUFFER_FLAG_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>
#include <LLGL-C/Format.h>
#include <LLGL-C/VertexAttribute.h>


/* ----- Structures ----- */

typedef struct LLGLBufferDescriptor
{
    uint64_t                    size;
    uint32_t                    stride;
    LLGLFormat                  format;
    long                        bindFlags;
    long                        cpuAccessFlags;
    long                        miscFlags;
    size_t                      numVertexAttribs;
    const LLGLVertexAttribute*  vertexAttribs;
}
LLGLBufferDescriptor;

typedef struct LLGLBufferViewDescriptor
{
    LLGLFormat  format;
    uint64_t    offset;
    uint64_t    size;
}
LLGLBufferViewDescriptor;


#endif



// ================================================================================
