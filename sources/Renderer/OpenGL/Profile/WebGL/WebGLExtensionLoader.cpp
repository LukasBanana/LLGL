/*
 * WebGLExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Ext/GLExtensionLoader.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "WebGL.h"
#include "../../GLCore.h"
#if defined(LLGL_OS_WASM)
#   include <EGL/egl.h>
#endif
#include <set>


namespace LLGL
{


// Global member to store if the extension have already been loaded
static bool                     g_WebGLExtensionsLoaded = false;
static std::set<const char*>    g_supportedWebGLExtensions;
static std::set<const char*>    g_loadedWebGLExtensions;

static void EnableWebGLExtension(GLExt ext, const char* name)
{
    RegisterExtension(ext);
    g_supportedWebGLExtensions.insert(name); //TODO: find better way to determine supported GLES extensions
    g_loadedWebGLExtensions.insert(name);
}

bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_WebGLExtensionsLoaded)
        return true;

    #define ENABLE_GLEXT(NAME) \
        EnableWebGLExtension(GLExt::NAME, "GL_" #NAME)

    const int version = GLGetVersion();

    ENABLE_GLEXT(ARB_clear_buffer_object);
    ENABLE_GLEXT(ARB_clear_texture);
    ENABLE_GLEXT(ARB_buffer_storage);
    ENABLE_GLEXT(ARB_copy_buffer);
    ENABLE_GLEXT(ARB_draw_buffers);
    ENABLE_GLEXT(ARB_draw_buffers_blend);
    ENABLE_GLEXT(ARB_draw_elements_base_vertex);
    ENABLE_GLEXT(ARB_draw_instanced);
    ENABLE_GLEXT(ARB_draw_indirect);
    ENABLE_GLEXT(ARB_framebuffer_object);
    ENABLE_GLEXT(ARB_geometry_shader4);               // no procedures
    ENABLE_GLEXT(ARB_instanced_arrays);
    ENABLE_GLEXT(ARB_internalformat_query);
    ENABLE_GLEXT(ARB_internalformat_query2);
    ENABLE_GLEXT(ARB_multitexture);
    ENABLE_GLEXT(ARB_multi_draw_indirect);
    ENABLE_GLEXT(ARB_occlusion_query);
    ENABLE_GLEXT(ARB_pipeline_statistics_query);
    ENABLE_GLEXT(ARB_polygon_offset_clamp);
    ENABLE_GLEXT(ARB_sampler_objects);
    ENABLE_GLEXT(ARB_seamless_cubemap_per_texture);
    ENABLE_GLEXT(ARB_shader_image_load_store);
    ENABLE_GLEXT(ARB_shader_objects);
    ENABLE_GLEXT(ARB_shader_objects_21);
    ENABLE_GLEXT(ARB_sync);
    ENABLE_GLEXT(ARB_texture_compression);
    ENABLE_GLEXT(ARB_texture_cube_map);               // no procedures
    ENABLE_GLEXT(ARB_texture_cube_map_array);         // no procedures
    ENABLE_GLEXT(ARB_texture_multisample);
    ENABLE_GLEXT(ARB_texture_storage);
    ENABLE_GLEXT(ARB_texture_storage_multisample);
    ENABLE_GLEXT(ARB_timer_query);
    ENABLE_GLEXT(ARB_transform_feedback3);
    ENABLE_GLEXT(ARB_uniform_buffer_object);
    ENABLE_GLEXT(ARB_vertex_array_object);
    ENABLE_GLEXT(ARB_vertex_buffer_object);
    ENABLE_GLEXT(ARB_vertex_shader);
    ENABLE_GLEXT(ARB_viewport_array);
    ENABLE_GLEXT(ARB_ES2_compatibility);
    ENABLE_GLEXT(ARB_compatibility);
    ENABLE_GLEXT(ARB_map_buffer_range);

    ENABLE_GLEXT(EXT_blend_color);
    ENABLE_GLEXT(EXT_blend_equation_separate);
    ENABLE_GLEXT(EXT_blend_func_separate);
    ENABLE_GLEXT(EXT_blend_minmax);
    ENABLE_GLEXT(EXT_copy_texture);
    ENABLE_GLEXT(EXT_draw_buffers2);
    ENABLE_GLEXT(EXT_gpu_shader4);
    ENABLE_GLEXT(EXT_stencil_two_side);
    ENABLE_GLEXT(EXT_texture3D);
    ENABLE_GLEXT(EXT_texture_array);
    ENABLE_GLEXT(EXT_transform_feedback);

    // GLES 3.0
    ENABLE_GLEXT(ARB_ES3_compatibility);
    ENABLE_GLEXT(ARB_get_program_binary);
    ENABLE_GLEXT(ARB_shader_objects_30);

    #undef ENABLE_GLEXT

    return true;
}

bool AreOpenGLExtensionsLoaded()
{
    return g_WebGLExtensionsLoaded;
}

const std::set<const char*>& GetSupportedOpenGLExtensions()
{
    return g_supportedWebGLExtensions;
}

const std::set<const char*>& GetLoadedOpenGLExtensions()
{
    return g_loadedWebGLExtensions;
}


} // /namespace LLGL



// ================================================================================
