/*
 * OpenGL.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_H
#define LLGL_OPENGL_H


#if LLGL_OPENGL
#   include "Profile/GLCore/OpenGLCore.h"
#elif LLGL_OPENGLES3
#   include "Profile/GLES/OpenGLES.h"
#elif LLGL_WEBGL
#   include "Profile/WebGL/WebGL.h"
#else
#   error Unknown OpenGL backend
#endif


#if LLGL_WEBGL

#define LLGL_GLEXT_DRAW_INSTANCED 1
#define LLGL_GLEXT_UNIFORM_BUFFER_OBJECT 1
#define LLGL_GLEXT_SAMPLER_OBJECTS 1
#define LLGL_GLEXT_TRANSFORM_FEEDBACK 1
#define LLGL_GLEXT_VERTEX_ARRAY_OBJECT 1
#define LLGL_GLEXT_FRAMEBUFFER_OBJECT 1
#define LLGL_GLEXT_SHADER_OBJECTS_30 1

#else // LLGL_WEBGL

#if GL_ARB_framebuffer_object || GL_EXT_framebuffer_object || GL_ES_VERSION_2_0 || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_FRAMEBUFFER_OBJECT 1
#endif

#if !LLGL_GL_ENABLE_OPENGL2X

#if GL_ARB_vertex_array_object || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_VERTEX_ARRAY_OBJECT 1
#endif

#if GL_ARB_sampler_objects || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_SAMPLER_OBJECTS 1
#endif

#if GL_ARB_draw_instanced || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_DRAW_INSTANCED 1
#endif

#if GL_ARB_separate_shader_objects
#   define LLGL_GLEXT_SEPARATE_SHADER_OBJECTS 1
#endif

#if GL_ARB_uniform_buffer_object || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_UNIFORM_BUFFER_OBJECT 1
#endif

#if GL_ARB_polygon_offset_clamp
#   define LLGL_GLEXT_POLYGON_OFFSET_CLAMP 1
#endif

#if GL_ARB_texture_multisample || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_TEXTURE_MULTISAMPLE 1
#endif

#if GL_ARB_gl_spirv && GL_ARB_ES2_compatibility
#   define LLGL_GLEXT_GL_SPIRV 1
#endif

#if GL_ARB_get_texture_sub_image
#   define LLGL_GLEXT_GET_TEXTURE_SUB_IMAGE 1
#endif

#if GL_ARB_viewport_array
#   define LLGL_GLEXT_VIEWPORT_ARRAY 1
#endif

#if GL_ARB_multi_bind
#   define LLGL_GLEXT_MULTI_BIND 1
#endif

#if GL_ARB_shader_image_load_store || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_SHADER_IMAGE_LOAD_STORE 1
#endif

#if GL_ARB_draw_indirect || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_DRAW_INDIRECT 1
#endif

#if GL_ARB_draw_elements_base_vertex || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX 1
#endif

#if GL_ARB_framebuffer_no_attachments || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_FRAMEBUFFER_NO_ATTACHMENTS 1
#endif

#if GL_ARB_base_instance
#   define LLGL_GLEXT_BASE_INSTANCE 1
#endif

#if GL_ARB_multi_draw_indirect
#   define LLGL_GLEXT_MULTI_DRAW_INDIRECT 1
#endif

#if GL_ARB_compute_shader || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_COMPUTE_SHADER 1
#endif

#if GL_KHR_debug || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DEBUG 1
#endif

//TODO: which extension?
#if defined LLGL_OPENGL && !LLGL_GL_ENABLE_OPENGL2X
#   define LLGL_GLEXT_CONDITIONAL_RENDER 1
#endif

#if GL_ES_VERSION_3_0 || defined __APPLE__
#   define LLGL_GLEXT_TRANSFORM_FEEDBACK 1
#endif

#if GL_ARB_transform_feedback2
#   define LLGL_GLEXT_TRNASFORM_FEEDBACK2 1
#endif

#if GL_EXT_draw_buffers_indexed || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS_INDEXED 1
#endif

#if GL_ARB_draw_buffers_blend || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS_BLEND 1
#endif

#if GL_ARB_tessellation_shader || GL_ES_VERSION_3_2
#   define LLGL_GLEXT_TESSELLATION_SHADER 1
#endif

#if GL_ARB_shader_storage_buffer_object || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT 1
#endif

#if GL_ARB_texture_view
#   define LLGL_GLEXT_TEXTURE_VIEW 1
#endif

#if GL_ARB_direct_state_access && LLGL_GL_ENABLE_DSA_EXT
#   define LLGL_GLEXT_DIRECT_STATE_ACCESS 1
#endif

#if GL_ARB_get_program_binary || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_GET_PROGRAM_BINARY 1
#endif

#if GL_ARB_texture_storage || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_TEXTURE_STORAGE 1
#endif

#if GL_ARB_program_interface_query || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_PROGRAM_INTERFACE_QUERY
#endif

#if LLGL_OPENGL || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER 1
#endif

#if GL_ARB_clip_control
#   define LLGL_GLEXT_CLIP_CONTROL 1
#endif

#if GL_ARB_texture_storage_multisample && !LLGL_GL_ENABLE_OPENGL2X
#   define LLGL_GLEXT_TEXTURE_STORAGE_MULTISAMPLE 1
#endif

#if GL_TEXTURE_BORDER_COLOR
#   define LLGL_SAMPLER_BORDER_COLOR 1
#endif

#if GL_ARB_shader_image_load_store || GL_ES_VERSION_3_1
#   define LLGL_GLEXT_MEMORY_BARRIERS 1
#endif

// At most one of these should be defined to indicate which API
// we'll be using to implement fixed-index primitive restart.
#if GL_ES_VERSION_2_0 || GL_VERSION_4_3
#   define LLGL_PRIMITIVE_RESTART_FIXED_INDEX 1
#elif GL_VERSION_3_1
#   define LLGL_PRIMITIVE_RESTART 1
#endif

#if GL_VERSION_3_0 || GL_ES_VERSION_3_0
#   define LLGL_GLEXT_SHADER_OBJECTS_30 1
#endif

#if GL_VERSION_4_0
#   define LLGL_GLEXT_SHADER_OBJECTS_40 1
#endif

#endif // !LLGL_GL_ENABLE_OPENGL2X

// WebGL does not support independent stencil face values, even though glStencilFuncSeparate() is provided.
// Since D3D does not support independent stencil faces, WebGL cannot emulate it in ANGLE or other web rendering abstraction layers.
#define LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES 1

#endif // /LLGL_WEBGL

// GL on macOS, GLES on iOS, and WebGL always require a fragment shader in a shader program.
#if defined __APPLE__ || LLGL_WEBGL
#   define LLGL_USE_NULL_FRAGMENT_SHADER 1
#endif

#endif



// ================================================================================
