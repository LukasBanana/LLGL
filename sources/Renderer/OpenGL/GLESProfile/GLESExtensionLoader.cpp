/*
 * GLESExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Ext/GLExtensionLoader.h"
#include "../Ext/GLExtensionRegistry.h"
#include "GLESExtensions.h"
#include "GLESExtensionsProxy.h"
#include "OpenGLES.h"
#include "../GLCore.h"
#if defined(LLGL_OS_IOS)
//#   import <OpenGLES/EAGL.h>
#else
#   include <EGL/egl.h>
#endif
#include <LLGL/Utils/ForRange.h>
#include <functional>
#include <string>
#include <map>


namespace LLGL
{


// OpenGLES extension map type: Maps the extension name to boolean indicating whether or not the extension was loaded successully.
using GLESExtensionMap = std::map<std::string, bool>;

/* --- Internal functions --- */

#ifndef LLGL_OS_IOS

template <typename T>
bool LoadGLProc(T& procAddr, const char* procName)
{
    /* Load OpenGLES procedure address with EGL */
    procAddr = eglGetProcAddress(procName);
    return (procAddr != nullptr);
}


using LoadGLExtensionProc = std::function<bool(const char* extName, bool abortOnFailure, bool usePlaceholder)>;

#define DECL_LOADGLEXT_PROC(EXTNAME) \
    Load_ ## EXTNAME(const char* extName, bool abortOnFailure, bool usePlaceholder)

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#define LOAD_GLPROC(NAME)                                                               \
    if (usePlaceholder)                                                                 \
    {                                                                                   \
        NAME = Proxy_##NAME;                                                            \
    }                                                                                   \
    else if (!LoadGLProc(NAME, #NAME))                                                  \
    {                                                                                   \
        if (abortOnFailure)                                                             \
            LLGL_TRAP("failed to load OpenGLES procedure: %s [%s]", #NAME, extName);    \
        return false;                                                                   \
    }

/* --- Common GLES extensions --- */

/*static bool DECL_LOADGLEXT_PROC(GL_OES_tessellation_shader)
{
    LOAD_GLPROC( glPatchParameteriOES );
    return true;
}

static bool DECL_LOADGLEXT_PROC(GL_ARB_compute_shader)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}*/

#undef DECL_LOADGLEXT_PROC
#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC


/* --- Common extension loading functions --- */

static GLESExtensionMap QuerySupportedOpenGLExtensions(bool coreProfile)
{
    GLESExtensionMap extensions;

    /* Get number of extensions */
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for_range(i, numExtensions)
    {
        /* Get current extension string */
        if (auto extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)))
            extensions[extString] = false;
    }

    return extensions;
}

#endif // /LLGL_OS_IOS

// Global member to store if the extension have already been loaded
static bool g_OpenGLExtensionsLoaded = false;

bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_OpenGLExtensionsLoaded)
        return true;

    #define ENABLE_GLEXT(NAME) \
        RegisterExtension(GLExt::NAME)

    const int version = GLGetVersion();

    ENABLE_GLEXT(ARB_clear_buffer_object);
    ENABLE_GLEXT(ARB_clear_texture);
    ENABLE_GLEXT(ARB_clip_control);
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

    if (version >= 300)
    {
        ENABLE_GLEXT(ARB_ES3_compatibility);
        ENABLE_GLEXT(ARB_get_program_binary);
        ENABLE_GLEXT(ARB_shader_objects_30);
    }

    if (version >= 310)
    {
        ENABLE_GLEXT(ARB_shader_storage_buffer_object);
        ENABLE_GLEXT(ARB_program_interface_query);
        ENABLE_GLEXT(ARB_compute_shader);
        ENABLE_GLEXT(ARB_framebuffer_no_attachments);
    }

    if (version >= 320)
    {
        ENABLE_GLEXT(ARB_tessellation_shader);
        ENABLE_GLEXT(ARB_copy_image);
    }

    #undef ENABLE_GLEXT

    #if 0 //TODO

    GLESExtensionMap extensions = QuerySupportedOpenGLExtensions(isCoreProfile);

    auto LoadExtension = [&extensions, abortOnFailure](const char* extName, const LoadGLExtensionProc& extLoadingProc, GLExt extensionID) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extensions.find(extName);
        if (it != extensions.end())
        {
            if (extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ false))
            {
                /* Enable extension in registry */
                RegisterExtension(extensionID);
                it->second = true;
            }
            else
            {
                /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
                extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ true);
            }
        }
        else
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ true);
        }
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" #NAME, Load_GL_##NAME, GLExt::NAME)

    /* Load hardware buffer extensions */
    //LOAD_GLEXT( OES_tessellation_shader );
    //LOAD_GLEXT( ARB_compute_shader      );

    #undef LOAD_GLEXT

    #endif // /TODO

    g_OpenGLExtensionsLoaded = true;

    return true;
}

bool AreOpenGLExtensionsLoaded()
{
    return g_OpenGLExtensionsLoaded;
}


} // /namespace LLGL



// ================================================================================
