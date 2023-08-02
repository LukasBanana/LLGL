/*
 * PipelineStateFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_PIPELINE_STATE_FLAGS_H
#define LLGL_C99_PIPELINE_STATE_FLAGS_H


#include <LLGL/StaticLimits.h>
#include <LLGL-C/Format.h>
#include <stdint.h>
#include <stdbool.h>


/* ----- Enumerations ----- */

typedef enum LLGLPrimitiveTopology
{
    LLGLPrimitiveTopologyPointList,
    LLGLPrimitiveTopologyLineList,
    LLGLPrimitiveTopologyLineStrip,
    LLGLPrimitiveTopologyLineListAdjacency,
    LLGLPrimitiveTopologyLineStripAdjacency,
    LLGLPrimitiveTopologyTriangleList,
    LLGLPrimitiveTopologyTriangleStrip,
    LLGLPrimitiveTopologyTriangleListAdjacency,
    LLGLPrimitiveTopologyTriangleStripAdjacency,
    LLGLPrimitiveTopologyPatches1,
    LLGLPrimitiveTopologyPatches2,
    LLGLPrimitiveTopologyPatches3,
    LLGLPrimitiveTopologyPatches4,
    LLGLPrimitiveTopologyPatches5,
    LLGLPrimitiveTopologyPatches6,
    LLGLPrimitiveTopologyPatches7,
    LLGLPrimitiveTopologyPatches8,
    LLGLPrimitiveTopologyPatches9,
    LLGLPrimitiveTopologyPatches10,
    LLGLPrimitiveTopologyPatches11,
    LLGLPrimitiveTopologyPatches12,
    LLGLPrimitiveTopologyPatches13,
    LLGLPrimitiveTopologyPatches14,
    LLGLPrimitiveTopologyPatches15,
    LLGLPrimitiveTopologyPatches16,
    LLGLPrimitiveTopologyPatches17,
    LLGLPrimitiveTopologyPatches18,
    LLGLPrimitiveTopologyPatches19,
    LLGLPrimitiveTopologyPatches20,
    LLGLPrimitiveTopologyPatches21,
    LLGLPrimitiveTopologyPatches22,
    LLGLPrimitiveTopologyPatches23,
    LLGLPrimitiveTopologyPatches24,
    LLGLPrimitiveTopologyPatches25,
    LLGLPrimitiveTopologyPatches26,
    LLGLPrimitiveTopologyPatches27,
    LLGLPrimitiveTopologyPatches28,
    LLGLPrimitiveTopologyPatches29,
    LLGLPrimitiveTopologyPatches30,
    LLGLPrimitiveTopologyPatches31,
    LLGLPrimitiveTopologyPatches32,
}
LLGLPrimitiveTopology;

typedef enum LLGLCompareOp
{
    LLGLCompareOpNeverPass,
    LLGLCompareOpLess,
    LLGLCompareOpEqual,
    LLGLCompareOpLessEqual,
    LLGLCompareOpGreater,
    LLGLCompareOpNotEqual,
    LLGLCompareOpGreaterEqual,
    LLGLCompareOpAlwaysPass,
}
LLGLCompareOp;

typedef enum LLGLStencilOp
{
    LLGLStencilOpKeep,
    LLGLStencilOpZero,
    LLGLStencilOpReplace,
    LLGLStencilOpIncClamp,
    LLGLStencilOpDecClamp,
    LLGLStencilOpInvert,
    LLGLStencilOpIncWrap,
    LLGLStencilOpDecWrap,
}
LLGLStencilOp;

typedef enum LLGLBlendOp
{
    LLGLBlendOpZero,
    LLGLBlendOpOne,
    LLGLBlendOpSrcColor,
    LLGLBlendOpInvSrcColor,
    LLGLBlendOpSrcAlpha,
    LLGLBlendOpInvSrcAlpha,
    LLGLBlendOpDstColor,
    LLGLBlendOpInvDstColor,
    LLGLBlendOpDstAlpha,
    LLGLBlendOpInvDstAlpha,
    LLGLBlendOpSrcAlphaSaturate,
    LLGLBlendOpBlendFactor,
    LLGLBlendOpInvBlendFactor,
    LLGLBlendOpSrc1Color,
    LLGLBlendOpInvSrc1Color,
    LLGLBlendOpSrc1Alpha,
    LLGLBlendOpInvSrc1Alpha,
}
LLGLBlendOp;

typedef enum LLGLBlendArithmetic
{
    LLGLBlendArithmeticAdd,
    LLGLBlendArithmeticSubtract,
    LLGLBlendArithmeticRevSubtract,
    LLGLBlendArithmeticMin,
    LLGLBlendArithmeticMax,
}
LLGLBlendArithmetic;

typedef enum LLGLPolygonMode
{
    LLGLPolygonModeFill,
    LLGLPolygonModeWireframe,
    LLGLPolygonModePoints,
}
LLGLPolygonMode;

typedef enum LLGLCullMode
{
    LLGLCullModeDisabled,
    LLGLCullModeFront,
    LLGLCullModeBack,
}
LLGLCullMode;

typedef enum LLGLLogicOp
{
    LLGLLogicOpDisabled,
    LLGLLogicOpClear,
    LLGLLogicOpSet,
    LLGLLogicOpCopy,
    LLGLLogicOpCopyInverted,
    LLGLLogicOpNoOp,
    LLGLLogicOpInvert,
    LLGLLogicOpAND,
    LLGLLogicOpANDReverse,
    LLGLLogicOpANDInverted,
    LLGLLogicOpNAND,
    LLGLLogicOpOR,
    LLGLLogicOpORReverse,
    LLGLLogicOpORInverted,
    LLGLLogicOpNOR,
    LLGLLogicOpXOR,
    LLGLLogicOpEquiv,
}
LLGLLogicOp;

typedef enum LLGLTessellationPartition
{
    LLGLTessellationPartitionUndefined,
    LLGLTessellationPartitionInteger,
    LLGLTessellationPartitionPow2,
    LLGLTessellationPartitionFractionalOdd,
    LLGLTessellationPartitionFractionalEven,
}
LLGLTessellationPartition;


/* ----- Flags ----- */

enum LLGLColorMaskFlags
{
    LLGLColorMaskZero   = 0,
    LLGLColorMaskR      = (1 << 0),
    LLGLColorMaskG      = (1 << 1),
    LLGLColorMaskB      = (1 << 2),
    LLGLColorMaskA      = (1 << 3),
    LLGLColorMaskAll    = (LLGLColorMaskR | LLGLColorMaskG | LLGLColorMaskB | LLGLColorMaskA),
};


/* ----- Structures ----- */

typedef struct LLGLViewport
{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
}
LLGLViewport;

typedef struct LLGLScissor
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
}
LLGLScissor;

typedef struct LLGLDepthDescriptor
{
    bool            testEnabled;
    bool            writeEnabled;
    LLGLCompareOp   compareOp;
}
LLGLDepthDescriptor;

typedef struct LLGLStencilFaceDescriptor
{
    LLGLStencilOp   stencilFailOp;
    LLGLStencilOp   depthFailOp;
    LLGLStencilOp   depthPassOp;
    LLGLCompareOp   compareOp;
    uint32_t        readMask;
    uint32_t        writeMask;
    uint32_t        reference;
}
LLGLStencilFaceDescriptor;

typedef struct LLGLStencilDescriptor
{
    bool                        testEnabled;
    bool                        referenceDynamic;
    LLGLStencilFaceDescriptor   front;
    LLGLStencilFaceDescriptor   back;
}
LLGLStencilDescriptor;

typedef struct LLGLDepthBiasDescriptor
{
    float constantFactor;
    float slopeFactor;
    float clamp;
}
LLGLDepthBiasDescriptor;

typedef struct LLGLRasterizerDescriptor
{
    LLGLPolygonMode         polygonMode;
    LLGLCullMode            cullMode;
    LLGLDepthBiasDescriptor depthBias;
    bool                    frontCCW;
    bool                    discardEnabled;
    bool                    depthClampEnabled;
    bool                    scissorTestEnabled;
    bool                    multiSampleEnabled;
    bool                    antiAliasedLineEnabled;
    bool                    conservativeRasterization;
    float                   lineWidth;
}
LLGLRasterizerDescriptor;

typedef struct LLGLBlendTargetDescriptor
{
    bool                blendEnabled;
    LLGLBlendOp         srcColor;
    LLGLBlendOp         dstColor;
    LLGLBlendArithmetic colorArithmetic;
    LLGLBlendOp         srcAlpha;
    LLGLBlendOp         dstAlpha;
    LLGLBlendArithmetic alphaArithmetic;
    uint8_t             colorMask;
}
LLGLBlendTargetDescriptor;

typedef struct LLGLBlendDescriptor
{
    bool                        alphaToCoverageEnabled;
    bool                        independentBlendEnabled;
    uint32_t                    sampleMask;
    LLGLLogicOp                 logicOp;
    float                       blendFactor[4];
    bool                        blendFactorDynamic;
    LLGLBlendTargetDescriptor   targets[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
}
LLGLBlendDescriptor;

typedef struct LLGLTessellationDescriptor
{
    LLGLTessellationPartition   partition;
    LLGLFormat                  indexFormat;
    uint32_t                    maxTessFactor;
    bool                        outputWindingCCW;
}
LLGLTessellationDescriptor;

typedef struct LLGLGraphicsPipelineDescriptor
{
    LLGLPipelineLayout          pipelineLayout;
    LLGLRenderPass              renderPass;
    LLGLShader                  vertexShader;
    LLGLShader                  tessControlShader;
    LLGLShader                  tessEvaluationShader;
    LLGLShader                  geometryShader;
    LLGLShader                  fragmentShader;
    LLGLPrimitiveTopology       primitiveTopology;
    size_t                      numViewports;
    const LLGLViewport*         viewports;
    size_t                      numScissors;
    const LLGLScissor*          scissors;
    LLGLDepthDescriptor         depth;
    LLGLStencilDescriptor       stencil;
    LLGLRasterizerDescriptor    rasterizer;
    LLGLBlendDescriptor         blend;
    LLGLTessellationDescriptor  tessellation;
}
LLGLGraphicsPipelineDescriptor;

typedef struct LLGLComputePipelineDescriptor
{
    LLGLPipelineLayout  pipelineLayout;
    LLGLShader          computeShader;
}
LLGLComputePipelineDescriptor;


#endif



// ================================================================================
