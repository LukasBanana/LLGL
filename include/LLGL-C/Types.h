/*
 * Types.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_TYPES_H
#define LLGL_C99_TYPES_H


#define LLGL_NULL_OBJECT        { NULL }
#define LLGL_GET(OBJ)           ((OBJ).internal)
#define LLGL_GET_AS(TYPE, OBJ)  (TYPE){ LLGL_GET(OBJ) }
#define LLGL_NULLABLE(IDENT)    IDENT

#define LLGL_DECL_WRAPPER_TYPE(NAME) \
    typedef struct NAME { void* internal; } NAME

#define LLGL_DECL_CONST_WRAPPER_TYPE(NAME) \
    typedef struct NAME { const void* internal; } NAME

LLGL_DECL_WRAPPER_TYPE( LLGLBuffer );
LLGL_DECL_WRAPPER_TYPE( LLGLBufferArray );
LLGL_DECL_WRAPPER_TYPE( LLGLCanvas );
LLGL_DECL_WRAPPER_TYPE( LLGLCommandBuffer );
LLGL_DECL_WRAPPER_TYPE( LLGLCommandQueue );
LLGL_DECL_WRAPPER_TYPE( LLGLDisplay );
LLGL_DECL_WRAPPER_TYPE( LLGLFence );
LLGL_DECL_WRAPPER_TYPE( LLGLImage );
LLGL_DECL_WRAPPER_TYPE( LLGLPipelineLayout );
LLGL_DECL_WRAPPER_TYPE( LLGLPipelineState );
LLGL_DECL_WRAPPER_TYPE( LLGLQueryHeap );
LLGL_DECL_WRAPPER_TYPE( LLGLRenderPass );
/*LLGL_DECL_WRAPPER_TYPE( LLGLRenderSystem );*/
LLGL_DECL_WRAPPER_TYPE( LLGLRenderTarget );
LLGL_DECL_WRAPPER_TYPE( LLGLRenderingProfiler );
LLGL_DECL_WRAPPER_TYPE( LLGLRenderingDebugger );
LLGL_DECL_WRAPPER_TYPE( LLGLResource );
LLGL_DECL_WRAPPER_TYPE( LLGLResourceHeap );
LLGL_DECL_WRAPPER_TYPE( LLGLSampler );
LLGL_DECL_WRAPPER_TYPE( LLGLShader );
LLGL_DECL_WRAPPER_TYPE( LLGLSurface );
LLGL_DECL_WRAPPER_TYPE( LLGLSwapChain );
LLGL_DECL_WRAPPER_TYPE( LLGLTexture );
LLGL_DECL_WRAPPER_TYPE( LLGLWindow );

LLGL_DECL_CONST_WRAPPER_TYPE( LLGLReport );

#undef LLGL_DECL_WRAPPER_TYPE
#undef LLGL_DECL_CONST_WRAPPER_TYPE


#endif



// ================================================================================
