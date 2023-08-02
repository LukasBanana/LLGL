/*
 * VertexAttribute.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_VERTEX_ATTRIBUTE_H
#define LLGL_C99_VERTEX_ATTRIBUTE_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Format.h>
#include <LLGL-C/SystemValue.h>
#include <stdint.h>


/* ----- Structures ----- */

typedef struct LLGLVertexAttribute
{
    const char*     name;
    LLGLFormat      format;
    uint32_t        location;
    uint32_t        semanticIndex;
    LLGLSystemValue systemValue;
    uint32_t        slot;
    uint32_t        offset;
    uint32_t        stride;
    uint32_t        instanceDivisor;
}
LLGLVertexAttribute;


#endif



// ================================================================================
