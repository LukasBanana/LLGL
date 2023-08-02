/*
 * QueryHeapFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_QUERY_HEAP_FLAGS_H
#define LLGL_C99_QUERY_HEAP_FLAGS_H


#include <stdint.h>
#include <stdbool.h>


typedef enum LLGLQueryType
{
    LLGLQueryTypeSamplesPassed,
    LLGLQueryTypeAnySamplesPassed,
    LLGLQueryTypeAnySamplesPassedConservative,
    LLGLQueryTypeTimeElapsed,
    LLGLQueryTypeStreamOutPrimitivesWritten,
    LLGLQueryTypeStreamOutOverflow,
    LLGLQueryTypePipelineStatistics,
}
LLGLQueryType;


/* ----- Structures ----- */

typedef struct LLGLQueryPipelineStatistics
{
    uint64_t inputAssemblyVertices;
    uint64_t inputAssemblyPrimitives;
    uint64_t vertexShaderInvocations;
    uint64_t geometryShaderInvocations;
    uint64_t geometryShaderPrimitives;
    uint64_t clippingInvocations;
    uint64_t clippingPrimitives;
    uint64_t fragmentShaderInvocations;
    uint64_t tessControlShaderInvocations;
    uint64_t tessEvaluationShaderInvocations;
    uint64_t computeShaderInvocations;
}
LLGLQueryPipelineStatistics;

typedef struct LLGLQueryHeapDescriptor
{
    LLGLQueryType   type;
    uint32_t        numQueries;
    bool            renderCondition;
}
LLGLQueryHeapDescriptor;


#endif



// ================================================================================
