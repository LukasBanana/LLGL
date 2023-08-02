/*
 * ShaderFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SHADER_FLAGS_H
#define LLGL_C99_SHADER_FLAGS_H


#include <LLGL-C/Types.h>
#include <LLGL-C/VertexAttribute.h>
#include <LLGL-C/FragmentAttribute.h>
#include <stddef.h>


/* ----- Enumerations ----- */

typedef enum LLGLShaderType
{
    LLGLShaderTypeUndefined,
    LLGLShaderTypeVertex,
    LLGLShaderTypeTessControl,
    LLGLShaderTypeTessEvaluation,
    LLGLShaderTypeGeometry,
    LLGLShaderTypeFragment,
    LLGLShaderTypeCompute,
}
LLGLShaderType;

typedef enum LLGLShaderSourceType
{
    LLGLShaderSourceTypeCodeString,
    LLGLShaderSourceTypeCodeFile,
    LLGLShaderSourceTypeBinaryBuffer,
    LLGLShaderSourceTypeBinaryFile,
}
LLGLShaderSourceType;


/* ----- Flags ----- */

enum LLGLShaderCompileFlags
{
    LLGLShaderCompileDebug                   = (1 << 0),
    LLGLShaderCompileNoOptimization          = (1 << 1),
    LLGLShaderCompileOptimizationLevel1      = (1 << 2),
    LLGLShaderCompileOptimizationLevel2      = (1 << 3),
    LLGLShaderCompileOptimizationLevel3      = (1 << 4),
    LLGLShaderCompileWarningsAreErrors       = (1 << 5),
    LLGLShaderCompilePatchClippingOrigin     = (1 << 6),
    LLGLShaderCompileSeparateShader          = (1 << 7),
};

enum LLGLStageFlags
{
    LLGLStageVertexStage         = (1 << 0),
    LLGLStageTessControlStage    = (1 << 1),
    LLGLStageTessEvaluationStage = (1 << 2),
    LLGLStageGeometryStage       = (1 << 3),
    LLGLStageFragmentStage       = (1 << 4),
    LLGLStageComputeStage        = (1 << 5),
    LLGLStageAllTessStages       = (LLGLStageTessControlStage | LLGLStageTessEvaluationStage),
    LLGLStageAllGraphicsStages   = (LLGLStageVertexStage | LLGLStageAllTessStages | LLGLStageGeometryStage | LLGLStageFragmentStage),
    LLGLStageAllStages           = (LLGLStageAllGraphicsStages | LLGLStageComputeStage),
};


/* ----- Structures ----- */

typedef struct LLGLShaderMacro
{
    const char* name;
    const char* definition;
}
LLGLShaderMacro;

typedef struct LLGLVertexShaderAttributes
{
    size_t                      numInputAttribs;
    const LLGLVertexAttribute*  inputAttribs;
    size_t                      numOutputAttribs;
    const LLGLVertexAttribute*  outputAttribs;
}
LLGLVertexShaderAttributes;

typedef struct LLGLFragmentShaderAttributes
{
    size_t                          numOutputAttribs;
    const LLGLFragmentAttribute*    outputAttribs;
}
LLGLFragmentShaderAttributes;

typedef struct LLGLComputeShaderAttributes
{
    LLGLExtent3D workGroupSize;
}
LLGLComputeShaderAttributes;

typedef struct LLGLShaderDescriptor
{
    LLGLShaderType                  type;
    const char*                     source;
    size_t                          sourceSize;
    LLGLShaderSourceType            sourceType;
    const char*                     entryPoint;
    const char*                     profile;
    const LLGLShaderMacro*          defines;
    long                            flags;
    LLGLVertexShaderAttributes      vertex;
    LLGLFragmentShaderAttributes    fragment;
    LLGLComputeShaderAttributes     compute;
}
LLGLShaderDescriptor;


#endif



// ================================================================================
