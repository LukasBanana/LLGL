/*
 * GLExtensionsEnum.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_EXTENSIONS_ENUM_H__
#define __LLGL_GL_EXTENSIONS_ENUM_H__


namespace LLGL
{


enum class GLExt
{
    /* Common extensions */
    EXT_blend_func_separate = 0,
    EXT_blend_minmax,
    EXT_blend_color,
    EXT_blend_equation_separate,
    ARB_draw_buffers_blend,
    EXT_draw_buffers2,
    ARB_multitexture,
    EXT_texture3D,
    ARB_clear_texture,
    ARB_texture_compression,
    ARB_texture_multisample,
    ARB_sampler_objects,
    ARB_multi_bind,
    ARB_vertex_buffer_object,
    ARB_instanced_arrays,
    ARB_draw_buffers,
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


} // /namespace LLGL


#endif



// ================================================================================
