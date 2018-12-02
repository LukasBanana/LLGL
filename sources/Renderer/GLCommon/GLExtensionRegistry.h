/*
 * GLExtensionRegistry.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_EXTENSION_REGISTRY_H
#define LLGL_GL_EXTENSION_REGISTRY_H


namespace LLGL
{


// OpenGL extension enumeration.
enum class GLExt
{
    /* Common extensions */
    EXT_blend_func_separate = 0,
    EXT_blend_minmax,
    EXT_blend_color,
    EXT_blend_equation_separate,
    ARB_draw_buffers,
    EXT_draw_buffers2,
    ARB_draw_buffers_blend,
    ARB_multitexture,
    EXT_texture3D,
    ARB_clear_texture,
    ARB_texture_compression,
    ARB_texture_multisample,
    ARB_sampler_objects,
    ARB_multi_bind,
    ARB_vertex_buffer_object,
    ARB_instanced_arrays,
    ARB_vertex_array_object,
    ARB_framebuffer_object,
    ARB_draw_instanced,
    ARB_draw_elements_base_vertex,
    ARB_base_instance,
    ARB_shader_objects,
    ARB_tessellation_shader,
    ARB_compute_shader,
    ARB_get_program_binary,
    ARB_program_interface_query,
    ARB_uniform_buffer_object,
    ARB_shader_storage_buffer_object,
    ARB_occlusion_query,
    NV_conditional_render,
    ARB_timer_query,
    ARB_viewport_array,
    EXT_stencil_two_side,//ATI_separate_stencil,
    KHR_debug,
    ARB_clip_control,
    EXT_transform_feedback,
    NV_transform_feedback,
    EXT_gpu_shader4,
    ARB_pipeline_statistics_query,
    ARB_sync,
    ARB_internalformat_query,
    ARB_internalformat_query2,
    ARB_ES2_compatibility,
    ARB_gl_spirv,
    ARB_texture_storage,
    ARB_texture_storage_multisample,
    ARB_buffer_storage,
    ARB_copy_buffer,
    ARB_polygon_offset_clamp,
    ARB_texture_view,
    ARB_shader_image_load_store,
    ARB_framebuffer_no_attachments,
    ARB_clear_buffer_object,
    ARB_draw_indirect,
    ARB_multi_draw_indirect,
    ARB_direct_state_access,

    /* Extensions without procedures */
    ARB_texture_cube_map,
    EXT_texture_array,
    ARB_texture_cube_map_array,
    ARB_geometry_shader4,
    NV_conservative_raster,
    INTEL_conservative_rasterization,

    /* Enumeration entry counter */
    Count,
};


// Registers the specified OpenGL extension support.
void RegisterExtension(GLExt extension);

// Returns true if the specified OpenGL extension is supported.
bool HasExtension(const GLExt extension);


} // /namespace LLGL


#endif



// ================================================================================
