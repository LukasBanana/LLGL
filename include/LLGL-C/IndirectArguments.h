/*
 * IndirectArguments.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_INDIRECT_ARGUMENTS_H
#define LLGL_C99_INDIRECT_ARGUMENTS_H


#include <stdint.h>


/* ----- Structures ----- */

typedef struct LLGLDrawIndirectArguments
{
    uint32_t    numVertices;
    uint32_t    numInstances;
    uint32_t    firstVertex;
    uint32_t    firstInstance;
}
LLGLDrawIndirectArguments;

typedef struct LLGLDrawIndexedIndirectArguments
{
    uint32_t    numIndices;
    uint32_t    numInstances;
    uint32_t    firstIndex;
    int32_t     vertexOffset;
    uint32_t    firstInstance;
}
LLGLDrawIndexedIndirectArguments;

typedef struct LLGLDrawPatchIndirectArguments
{
    uint32_t    numPatches;
    uint32_t    numInstances;
    uint32_t    firstPatch;
    uint32_t    firstInstance;
}
LLGLDrawPatchIndirectArguments;

typedef struct LLGLDispatchIndirectArguments
{
    uint32_t    numThreadGroups[3];
}
LLGLDispatchIndirectArguments;


#endif



// ================================================================================
