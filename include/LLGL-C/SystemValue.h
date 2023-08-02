/*
 * SystemValue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SYSTEM_VALUE_H
#define LLGL_C99_SYSTEM_VALUE_H


/* ----- Enumerations ----- */

typedef enum LLGLSystemValue
{
    LLGLSystemValueUndefined,
    LLGLSystemValueClipDistance,
    LLGLSystemValueColor,
    LLGLSystemValueCullDistance,
    LLGLSystemValueDepth,
    LLGLSystemValueDepthGreater,
    LLGLSystemValueDepthLess,
    LLGLSystemValueFrontFacing,
    LLGLSystemValueInstanceID,
    LLGLSystemValuePosition,
    LLGLSystemValuePrimitiveID,
    LLGLSystemValueRenderTargetIndex,
    LLGLSystemValueSampleMask,
    LLGLSystemValueSampleID,
    LLGLSystemValueStencil,
    LLGLSystemValueVertexID,
    LLGLSystemValueViewportIndex,
}
LLGLSystemValue;


#endif



// ================================================================================
