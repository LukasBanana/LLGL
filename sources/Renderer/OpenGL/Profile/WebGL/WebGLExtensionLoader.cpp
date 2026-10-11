/*
 * WebGLExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Ext/GLExtensionLoader.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "../../Platform/Wasm/WasmGLContext.h"
#include "../../GLCore.h"
#include "../../../CheckedCast.h"
#include "WebGL.h"
#include <set>
#include <string.h>
#include <functional>

#if defined(LLGL_OS_WASM)
#   include <emscripten.h>
#   include <emscripten/html5.h>
#   include <emscripten/html5_webgl.h>
#   include <EGL/egl.h>
#endif


/* ~~~~~ Define JavaScript wrapper functions for WebGL extensions that are not enabled by default in Emscripten ~~~~~ */

#define LOAD_WEBGLPROC(EXTENSION, NAME)                                 \
    EM_JS(int, LLGL_JS_Load_##NAME, (), {                               \
        const ctx = Module['ctx'];                                      \
        if (ctx) {                                                      \
            const ext = ctx.getExtension(#EXTENSION);                   \
            if (ext) {                                                  \
                const procName = #NAME;                                 \
                const webGLProcName = procName.startsWith("gl")         \
                    ? procName[2].toLowerCase() + procName.slice(3)     \
                    : procName;                                         \
                const proc = ext[webGLProcName];                        \
                if (proc)                                               \
                {                                                       \
                    ctx.LLGL_JS_##NAME = proc.bind(ext);                \
                    return 1;                                           \
                }                                                       \
            }                                                           \
        }                                                               \
        return 0;                                                       \
    });

#define DECL_GLPROC_void(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)        \
    LOAD_WEBGLPROC(EXTENSION, NAME);                                    \
    EM_JS(void, LLGL_JS_##NAME, PARAMS, {                               \
        Module['ctx'].LLGL_JS_##NAME ARGS;                              \
    })

#define DECL_GLPROC_GLboolean(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)   \
    LOAD_WEBGLPROC(EXTENSION, NAME);                                    \
    EM_JS(GLboolean, LLGL_JS_##NAME, PARAMS, {                          \
        return Module['ctx'].LLGL_JS_##NAME ARGS;                       \
    })

#define DECL_GLPROC(EXTENSION, PFNTYPE, NAME, RTYPE, PARAMS, ARGS)      \
    DECL_GLPROC_##RTYPE(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)

// Include inline header for object definitions
#include "WebGLExtensionsDecl.inl"

#undef DECL_GLPROC
#undef DECL_GLPROC_void
#undef DECL_GLPROC_GLboolean


namespace LLGL
{


// Global member to store if the extension have already been loaded
static bool                     g_WebGLExtensionsLoaded = false;

// WebGL specific extensions, e.g. "EXT_color_buffer_float".
// See https://developer.mozilla.org/en-US/docs/Web/API/EXT_color_buffer_float
static std::set<const char*>    g_supportedWebGLExtensions;


using LoadWebGLExtensionProc = std::function<bool(const char* extName, bool abortOnFailure, bool usePlaceholder)>;

#define DECL_LOADGLEXT_PROC(EXTNAME) \
    Load_GL_ ## EXTNAME(const char* extName, bool abortOnFailure, bool usePlaceholder)

#define LOAD_GLPROC(NAME)                                                           \
    if (usePlaceholder)                                                             \
    {                                                                               \
        NAME = Proxy_##NAME;                                                        \
    }                                                                               \
    else if (LLGL_JS_Load_##NAME() != 0)                                            \
    {                                                                               \
        NAME = &LLGL_JS_##NAME;                                                     \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        if (abortOnFailure)                                                         \
            LLGL_TRAP("failed to load WebGL procedure: %s [%s]", #NAME, extName);   \
        return false;                                                               \
    }

static bool DECL_LOADGLEXT_PROC(OES_draw_buffers_indexed)
{
    LOAD_GLPROC( glEnableiOES                );
    LOAD_GLPROC( glDisableiOES               );
    LOAD_GLPROC( glBlendEquationiOES         );
    LOAD_GLPROC( glBlendEquationSeparateiOES );
    LOAD_GLPROC( glBlendFunciOES             );
    LOAD_GLPROC( glBlendFuncSeparateiOES     );
    LOAD_GLPROC( glColorMaskiOES             );
  //LOAD_GLPROC( glIsEnablediOES             ); // Not supported in WebGL
    return true;
}

#undef DECL_LOADGLEXT_PROC
#undef LOAD_GLPROC


static void EnableWebGLExtension(GLExt ext, const char* name)
{
    RegisterExtension(ext);
    g_supportedWebGLExtensions.insert(name);
}

static bool IsWebGLExtensionSupported(const char* extensions, const char* name)
{
    return (::strstr(extensions, name) != nullptr);
}

static void EnableWebGLExtensionIfSupported(GLExt ext, const char* name, const char* extensions)
{
    if (IsWebGLExtensionSupported(extensions, name))
        EnableWebGLExtension(ext, name);
}

bool LoadSupportedOpenGLExtensions(GLContext& context, bool /*isCoreProfile*/, bool abortOnFailure)
{
    WasmGLContext& contextWasm = LLGL_CAST(WasmGLContext&, context);

    /* Only load GL extensions once */
    if (g_WebGLExtensionsLoaded)
        return true;

    char* extensions = emscripten_webgl_get_supported_extensions();
    LLGL_ASSERT(extensions != nullptr, "Failed to get supported WebGL extensions via `emscripten_webgl_get_supported_extensions()`");

    #define ENABLE_GLEXT(NAME) \
        EnableWebGLExtension(GLExt::NAME, "GL_" #NAME)

    #define ENABLE_GLEXT_IF_SUPPORTED(NAME) \
        EnableWebGLExtensionIfSupported(GLExt::NAME, "GL_" #NAME, extensions)
    
    auto LoadWebGLExtension = [extensions, abortOnFailure, &contextWasm](GLExt ext, const LoadWebGLExtensionProc& extLoadingProc, const char* extName) -> void
    {
        /* Explicitly enable the WebGL extension if it is supported */
        if (IsWebGLExtensionSupported(extensions, extName) && emscripten_webgl_enable_extension(contextWasm.GetWebGLContext(), extName))
        {
            if (extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ false))
                EnableWebGLExtension(ext, extName);
            else
                extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ true);
        }
    };

    #define LOAD_WEBGL_EXT_IF_SUPPORTED(NAME, WEBGL_NAME) \
        LoadWebGLExtension(GLExt::NAME, Load_GL_ ## WEBGL_NAME, #WEBGL_NAME)

    const int version = GLGetVersion();

    ENABLE_GLEXT(ARB_clear_buffer_object);
    ENABLE_GLEXT(ARB_clear_texture);
    ENABLE_GLEXT(ARB_buffer_storage);
    ENABLE_GLEXT(ARB_copy_buffer);
    ENABLE_GLEXT(ARB_draw_buffers);
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

    // WebGL specific extensions
    ENABLE_GLEXT_IF_SUPPORTED(EXT_texture_filter_anisotropic);
    ENABLE_GLEXT_IF_SUPPORTED(EXT_color_buffer_float);
    ENABLE_GLEXT_IF_SUPPORTED(EXT_color_buffer_half_float);

    // Load WebGL extensions dynamically
    LOAD_WEBGL_EXT_IF_SUPPORTED(ARB_draw_buffers_blend, OES_draw_buffers_indexed);

    #undef ENABLE_GLEXT
    #undef ENABLE_GLEXT_IF_SUPPORTED

    /* Free the allocated extension string as per Emscripten documentation */
    ::free(extensions);

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
    return GetSupportedOpenGLExtensions(); // Don't distinguish between supported and loaded extensions for WebGL backend
}


} // /namespace LLGL



// ================================================================================
