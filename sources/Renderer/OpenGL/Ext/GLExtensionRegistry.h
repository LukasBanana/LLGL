/*
 * GLExtensionRegistry.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_EXTENSION_REGISTRY_H
#define LLGL_GL_EXTENSION_REGISTRY_H


#include "../../../Core/Exception.h"


#define LLGL_ASSERT_GL_EXT(EXT, ...) \
    if (!LLGL::HasExtension(LLGL::GLExt::EXT)) { LLGL::TrapGLExtensionNotSupported(__FUNCTION__, "GL_" #EXT LLGL_VA_ARGS(__VA_ARGS__)); }


namespace LLGL
{


// OpenGL extension enumeration.
enum class GLExt
{
    /* OpenGL core extensions (ARB) */
    ARB_base_instance = 0,              // GL 4.1
    ARB_clear_buffer_object,
    ARB_clear_texture,
    ARB_clip_control,
    ARB_buffer_storage,
    ARB_compute_shader,                 // GL 4.2
    ARB_copy_buffer,                    // GL 3.1
    ARB_copy_image,                     // GL 4.3
    ARB_direct_state_access,            // GL 4.5
    ARB_draw_buffers,
    ARB_draw_buffers_blend,
    ARB_draw_elements_base_vertex,      // GL 3.1
    ARB_draw_instanced,                 // GL 2.1
    ARB_draw_indirect,
    ARB_framebuffer_no_attachments,     // GL 4.2
    ARB_framebuffer_object,
    ARB_get_program_binary,
    ARB_get_texture_sub_image,          // GL 4.5
    ARB_geometry_shader4,               // no procedures
    ARB_gl_spirv,                       // GL 4.6
    ARB_instanced_arrays,               // GL 2.1
    ARB_internalformat_query,
    ARB_internalformat_query2,
    ARB_multitexture,                   // GL 1.2
    ARB_multi_bind,                     // GL 4.3
    ARB_multi_draw_indirect,
    ARB_occlusion_query,
    ARB_pipeline_statistics_query,
    ARB_polygon_offset_clamp,
    ARB_program_interface_query,        // GL 4.2
    ARB_sampler_objects,                // GL 3.2
    ARB_seamless_cubemap_per_texture,   // GL 3.2
    ARB_shader_image_load_store,
    ARB_shader_objects,                 // GL 2.0
    ARB_shader_objects_21,              // GL 2.1
    ARB_shader_objects_30,              // GL 3.0
    ARB_shader_objects_40,              // GL 4.0
    ARB_shader_storage_buffer_object,   // GL 4.2
    ARB_sync,
    ARB_tessellation_shader,            // GL 3.2
    ARB_texture_compression,
    ARB_texture_cube_map,               // no procedures
    ARB_texture_cube_map_array,         // no procedures
    ARB_texture_multisample,
    ARB_texture_storage,
    ARB_texture_storage_multisample,
    ARB_texture_view,                   // GL 4.3
    ARB_timer_query,
    ARB_transform_feedback3,
    ARB_uniform_buffer_object,
    ARB_vertex_array_object,
    ARB_vertex_buffer_object,
    ARB_vertex_shader,
    ARB_viewport_array,
    ARB_ES2_compatibility,              // GL 4.0
    ARB_ES3_compatibility,              // GL 4.2
    ARB_compatibility,                  // GL 3.1
    ARB_map_buffer_range,               // GL 3.0
    ARB_separate_shader_objects,        // GL 4.1

    /* Khronos group extensions (KHR) */
    KHR_debug,

    /* Multi-vendor extensions (EXT) */
    EXT_blend_color,
    EXT_blend_equation_separate,
    EXT_blend_func_separate,
    EXT_blend_minmax,
    EXT_copy_texture,                   // GL 1.2
    EXT_draw_buffers2,
    EXT_gpu_shader4,                    // GL 2.0
    EXT_stencil_two_side,               //ATI_separate_stencil,
    EXT_texture3D,                      // GL 1.2
    EXT_texture_array,                  // no procedures
    EXT_transform_feedback,

    /* OpenGLES specific extensions (GLES) */
    OES_tessellation_shader,            // GLES 3.2

    /* NVIDIA specific extensions (NV) */
    NV_conditional_render,              //TODO: part of GL 3.0 core profile
    NV_conservative_raster,             // no procedures
    NV_transform_feedback,

    /* Intel sepcific extensions (INTEL) */
    INTEL_conservative_rasterization,   // no procedures

    /* Enumeration entry counter */
    Count,
};


// Registers the specified OpenGL extension support.
void RegisterExtension(GLExt extension);

// Returns true if the specified OpenGL extension is supported.
bool HasExtension(const GLExt extension);

// Returns ture if GL_ARB_sampler_objects is supported. Shortcut for 'HasExtension(GLExt::ARB_sampler_objects)'.
bool HasNativeSamplers();

// Returns true if GL_ARB_vertex_array_object is supported. Shortcut for 'HasExtension(GLExt::ARB_vertex_array_object)'.
bool HasNativeVAO();


} // /namespace LLGL


#endif



// ================================================================================
