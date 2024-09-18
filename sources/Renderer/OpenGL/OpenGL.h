/*
 * OpenGL.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_H
#define LLGL_OPENGL_H


#if LLGL_OPENGL
#   include "GLCoreProfile/OpenGLCore.h"
#elif LLGL_OPENGLES3
#   include "GLESProfile/OpenGLES.h"
#elif LLGL_WEBGL
#   include "WebGLProfile/WebGL.h"
#else
#   error Unknown OpenGL backend
#endif


//TODO: set this in CMake via option
#define LLGL_GL3PLUS_SUPPORTED 0

#if LLGL_WEBGL

#define LLGL_GLEXT_TRANSFORM_FEEDBACK

#else // LLGL_WEBGL

#if GL_ARB_draw_indirect || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_DRAW_INDIRECT
#endif

#if GL_ARB_draw_elements_base_vertex || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
#endif

#if GL_ARB_base_instance
#   define LLGL_GLEXT_BASE_INSTANCE
#endif

#if GL_ARB_multi_draw_indirect
#   define LLGL_GLEXT_MULTI_DRAW_INDIRECT
#endif

#if GL_ARB_compute_shader || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_COMPUTE_SHADER
#endif

#if GL_KHR_debug || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DEBUG 1
#endif

//TODO: which extension?
#if defined LLGL_OPENGL && LLGL_GL3PLUS_SUPPORTED
#   define LLGL_GLEXT_CONDITIONAL_RENDER
#endif

#if GL_ES_VERSION_3_0 || defined __APPLE__
#   define LLGL_GLEXT_TRANSFORM_FEEDBACK
#endif

#if GL_EXT_draw_buffers_indexed || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS_INDEXED
#endif

#if GL_ARB_draw_buffers_blend || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS_BLEND
#endif

#if GL_ARB_tessellation_shader || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_TESSELLATION_SHADER
#endif

#if GL_ARB_shader_storage_buffer_object || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT
#endif

#if GL_ARB_program_interface_query || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
#endif

#if LLGL_OPENGL || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER
#endif

#if GL_ARB_clip_control
#   define LLGL_GLEXT_CLIP_CONTROL
#endif

#if GL_TEXTURE_BORDER_COLOR
#   define LLGL_SAMPLER_BORDER_COLOR
#endif

#if GL_ARB_shader_image_load_store || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_MEMORY_BARRIERS
#endif

// At most one of these should be defined to indicate which API
// we'll be using to implement fixed-index primitive restart.
#if GL_ES_VERSION_2_0 || GL_VERSION_4_3
#   define LLGL_PRIMITIVE_RESTART_FIXED_INDEX 1
#elif GL_VERSION_3_1
#   define LLGL_PRIMITIVE_RESTART 1
#endif

// WebGL does not support independent stencil face values, even though glStencilFuncSeparate() is provided.
// Since D3D does not support independent stencil faces, WebGL cannot emulate it in ANGLE or other web rendering abstraction layers.
#define LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES 1

#endif // /LLGL_WEBGL

// GL on macOS, GLES on iOS, and WebGL always require a fragment shader in a shader program.
#if defined __APPLE__ || LLGL_WEBGL
#   define LLGL_USE_NULL_FRAGMENT_SHADER 1
#endif

#if !LLGL_GL3PLUS_SUPPORTED
#   define glGenFramebuffers                glGenFramebuffersEXT
#   define glDeleteFramebuffers             glDeleteFramebuffersEXT
#   define glBlitFramebuffer                glBlitFramebufferEXT
#   define glFramebufferTexture1D           glFramebufferTexture1DEXT
#   define glFramebufferTexture2D           glFramebufferTexture2DEXT
#   define glFramebufferTexture3D           glFramebufferTexture3DEXT
#   define glCheckFramebufferStatus         glCheckFramebufferStatusEXT
#   define glGenRenderbuffers               glGenRenderbuffersEXT
#   define glDeleteRenderbuffers            glDeleteRenderbuffersEXT
#   define glRenderbufferStorage            glRenderbufferStorageEXT
#   define glRenderbufferStorageMultisample glRenderbufferStorageMultisampleEXT
#   define glGetRenderbufferParameteriv     glGetRenderbufferParameterivEXT
#   define glGenerateMipmap                 glGenerateMipmapEXT
#endif


#endif



// ================================================================================
