/*
 * MacOSGLExt.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_GL_EXT_H
#define LLGL_MACOS_GL_EXT_H


#include "../../OpenGL.h"
#include "../../Ext/GLExtensions.h"


#ifndef GL_MIRROR_CLAMP_TO_EDGE
#define GL_MIRROR_CLAMP_TO_EDGE                         ( 0x8743 )
#endif

#ifndef GL_DEBUG_OUTPUT
#define GL_DEBUG_OUTPUT                                 ( 0x92E0 )
#endif

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#define GL_DEBUG_OUTPUT_SYNCHRONOUS                     ( 0x8242 )
#endif

#ifndef GL_PRIMITIVE_RESTART_FIXED_INDEX
#define GL_PRIMITIVE_RESTART_FIXED_INDEX                ( 0x8D69 )
#endif

#ifndef GL_ATOMIC_COUNTER_BUFFER
#define GL_ATOMIC_COUNTER_BUFFER                        ( 0x92C0 )
#endif

#ifndef GL_DISPATCH_INDIRECT_BUFFER
#define GL_DISPATCH_INDIRECT_BUFFER                     ( 0x90EE )
#endif

#ifndef GL_QUERY_BUFFER
#define GL_QUERY_BUFFER                                 ( 0x9192 )
#endif

#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER                        ( 0x90D2 )
#endif

#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER                               ( 0x91B9 )
#endif

#ifndef GL_ANY_SAMPLES_PASSED_CONSERVATIVE
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE              ( 0x8D6A )
#endif

#ifndef GL_MAX_COMPUTE_WORK_GROUP_COUNT
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT                 ( 0x91BE )
#endif

#ifndef GL_MAX_COMPUTE_WORK_GROUP_SIZE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE                  ( 0x91BF )
#endif

#ifndef GL_COMPARE_R_TO_TEXTURE
#define GL_COMPARE_R_TO_TEXTURE                         ( 0x884E )
#endif

#ifndef GL_CLAMP
#define GL_CLAMP                                        ( 0x2900 )
#endif

#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY                       ( 0x84FE )
#endif

#ifndef GL_CLIP_ORIGIN
#define GL_CLIP_ORIGIN                                  ( 0x935C )
#endif

#ifndef GL_CLIP_DEPTH_MODE
#define GL_CLIP_DEPTH_MODE                              ( 0x935D )
#endif

#ifndef GL_LOWER_LEFT
#define GL_LOWER_LEFT                                   ( 0x8CA1 )
#endif

#ifndef GL_UPPER_LEFT
#define GL_UPPER_LEFT                                   ( 0x8CA2 )
#endif

#ifndef GL_NEGATIVE_ONE_TO_ONE
#define GL_NEGATIVE_ONE_TO_ONE                          ( 0x935E )
#endif

#ifndef GL_ZERO_TO_ONE
#define GL_ZERO_TO_ONE                                  ( 0x935F )
#endif

// These macros are for MacOS X 10.6 support
// {

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                                  ( 0x8D40 )
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER                             ( 0x8CA8 )
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER                             ( 0x8CA9 )
#endif

#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE                         ( 0x8CD5 )
#endif

#ifndef GL_RENDERBUFFER_WIDTH
#define GL_RENDERBUFFER_WIDTH                           ( 0x8D42 )
#endif

#ifndef GL_RENDERBUFFER_HEIGHT
#define GL_RENDERBUFFER_HEIGHT                          ( 0x8D43 )
#endif

#ifndef GL_RENDERBUFFER_INTERNAL_FORMAT
#define GL_RENDERBUFFER_INTERNAL_FORMAT                 ( 0x8D44 )
#endif

#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT                             ( 0x8D00 )
#endif

#ifndef GL_STENCIL_ATTACHMENT
#define GL_STENCIL_ATTACHMENT                           ( 0x8D20 )
#endif

#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0                            ( 0x8CE0 )
#endif

#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1                            ( 0x8CE1 )
#endif

#ifndef GL_COLOR_ATTACHMENT2
#define GL_COLOR_ATTACHMENT2                            ( 0x8CE2 )
#endif

#ifndef GL_COLOR_ATTACHMENT3
#define GL_COLOR_ATTACHMENT3                            ( 0x8CE3 )
#endif

#ifndef GL_COLOR_ATTACHMENT4
#define GL_COLOR_ATTACHMENT4                            ( 0x8CE4 )
#endif

#ifndef GL_COLOR_ATTACHMENT5
#define GL_COLOR_ATTACHMENT5                            ( 0x8CE5 )
#endif

#ifndef GL_COLOR_ATTACHMENT6
#define GL_COLOR_ATTACHMENT6                            ( 0x8CE6 )
#endif

#ifndef GL_COLOR_ATTACHMENT7
#define GL_COLOR_ATTACHMENT7                            ( 0x8CE7 )
#endif

#ifndef GL_COLOR_ATTACHMENT8
#define GL_COLOR_ATTACHMENT8                            ( 0x8CE8 )
#endif

#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER                                 ( 0x8D41 )
#endif

#ifndef GL_FRAMEBUFFER_UNDEFINED
#define GL_FRAMEBUFFER_UNDEFINED                        ( 0x8219 )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT            ( 0x8CD6 )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT    ( 0x8CD7 )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER           ( 0x8CDB )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER           ( 0x8CDC )
#endif

#ifndef GL_FRAMEBUFFER_UNSUPPORTED
#define GL_FRAMEBUFFER_UNSUPPORTED                      ( 0x8CDD )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE           ( 0x8D56 )
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS         ( 0x8DA8 )
#endif

#ifndef GL_INVALID_FRAMEBUFFER_OPERATION
#define GL_INVALID_FRAMEBUFFER_OPERATION                ( 0x0506 )
#endif

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT                     ( 0x821A )
#endif

#ifndef GL_TEXTURE_1D_ARRAY
#define GL_TEXTURE_1D_ARRAY                             ( 0x8C18 )
#endif

#ifndef GL_PROXY_TEXTURE_1D_ARRAY
#define GL_PROXY_TEXTURE_1D_ARRAY                       ( 0x8C19 )
#endif

#ifndef GL_TEXTURE_2D_ARRAY
#define GL_TEXTURE_2D_ARRAY                             ( 0x8C1A )
#endif

#ifndef GL_PROXY_TEXTURE_2D_ARRAY
#define GL_PROXY_TEXTURE_2D_ARRAY                       ( 0x8C1B )
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY                       ( 0x9009 )
#endif

#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE                       ( 0x9100 )
#endif

#ifndef GL_PROXY_TEXTURE_2D_MULTISAMPLE
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE                 ( 0x9101 )
#endif

#ifndef GL_TEXTURE_2D_MULTISAMPLE_ARRAY
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY                 ( 0x9102 )
#endif

#ifndef GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY           ( 0x9103 )
#endif

#ifndef GL_MAP_READ_BIT
#define GL_MAP_READ_BIT                                 ( 0x0001 )
#endif

#ifndef GL_MAP_WRITE_BIT
#define GL_MAP_WRITE_BIT                                ( 0x0002 )
#endif

#ifndef GL_INVALID_INDEX
#define GL_INVALID_INDEX                                ( 0xFFFFFFFFu )
#endif

// } MacOS X 10.6 support


#endif



// ================================================================================
