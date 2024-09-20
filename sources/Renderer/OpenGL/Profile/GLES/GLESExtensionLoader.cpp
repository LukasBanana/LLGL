/*
 * GLESExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Ext/GLExtensionLoader.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "GLESExtensions.h"
#include "OpenGLES.h"
#include "../../GLCore.h"
#if defined(LLGL_OS_ANDROID)
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

#ifndef __APPLE__

template <typename T>
bool LoadGLProc(T& procAddr, const char* procName)
{
    /* Load OpenGLES procedure address with EGL */
    procAddr = reinterpret_cast<T>(eglGetProcAddress(procName));
    return (procAddr != nullptr);
}


using LoadGLExtensionProc = std::function<bool(const char* versionStr, bool abortOnFailure)>;

#define DECL_LOADGLEXT_PROC(EXTNAME) \
    Load_GL_ ## EXTNAME(const char* versionStr, bool abortOnFailure)

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#define LOAD_GLPROC(NAME)                                                               \
    if (!LoadGLProc(NAME, #NAME))                                                       \
    {                                                                                   \
        if (abortOnFailure)                                                             \
            LLGL_TRAP("failed to load OpenGLES procedure: %s [%s]", #NAME, versionStr); \
        return false;                                                                   \
    }

/* --- Common GLES extensions --- */

#if GL_ES_VERSION_3_1

static bool DECL_LOADGLEXT_PROC(GLES_3_1)
{
    LOAD_GLPROC( glDispatchCompute );
    LOAD_GLPROC( glDispatchComputeIndirect );
    LOAD_GLPROC( glDrawArraysIndirect );
    LOAD_GLPROC( glDrawElementsIndirect );
    LOAD_GLPROC( glFramebufferParameteri );
    LOAD_GLPROC( glGetFramebufferParameteriv );
    LOAD_GLPROC( glGetProgramInterfaceiv );
    LOAD_GLPROC( glGetProgramResourceIndex );
    LOAD_GLPROC( glGetProgramResourceName );
    LOAD_GLPROC( glGetProgramResourceiv );
    LOAD_GLPROC( glGetProgramResourceLocation );
    LOAD_GLPROC( glUseProgramStages );
    LOAD_GLPROC( glActiveShaderProgram );
    LOAD_GLPROC( glCreateShaderProgramv );
    LOAD_GLPROC( glBindProgramPipeline );
    LOAD_GLPROC( glDeleteProgramPipelines );
    LOAD_GLPROC( glGenProgramPipelines );
    LOAD_GLPROC( glIsProgramPipeline );
    LOAD_GLPROC( glGetProgramPipelineiv );
    LOAD_GLPROC( glProgramUniform1i );
    LOAD_GLPROC( glProgramUniform2i );
    LOAD_GLPROC( glProgramUniform3i );
    LOAD_GLPROC( glProgramUniform4i );
    LOAD_GLPROC( glProgramUniform1ui );
    LOAD_GLPROC( glProgramUniform2ui );
    LOAD_GLPROC( glProgramUniform3ui );
    LOAD_GLPROC( glProgramUniform4ui );
    LOAD_GLPROC( glProgramUniform1f );
    LOAD_GLPROC( glProgramUniform2f );
    LOAD_GLPROC( glProgramUniform3f );
    LOAD_GLPROC( glProgramUniform4f );
    LOAD_GLPROC( glProgramUniform1iv );
    LOAD_GLPROC( glProgramUniform2iv );
    LOAD_GLPROC( glProgramUniform3iv );
    LOAD_GLPROC( glProgramUniform4iv );
    LOAD_GLPROC( glProgramUniform1uiv );
    LOAD_GLPROC( glProgramUniform2uiv );
    LOAD_GLPROC( glProgramUniform3uiv );
    LOAD_GLPROC( glProgramUniform4uiv );
    LOAD_GLPROC( glProgramUniform1fv );
    LOAD_GLPROC( glProgramUniform2fv );
    LOAD_GLPROC( glProgramUniform3fv );
    LOAD_GLPROC( glProgramUniform4fv );
    LOAD_GLPROC( glProgramUniformMatrix2fv );
    LOAD_GLPROC( glProgramUniformMatrix3fv );
    LOAD_GLPROC( glProgramUniformMatrix4fv );
    LOAD_GLPROC( glProgramUniformMatrix2x3fv );
    LOAD_GLPROC( glProgramUniformMatrix3x2fv );
    LOAD_GLPROC( glProgramUniformMatrix2x4fv );
    LOAD_GLPROC( glProgramUniformMatrix4x2fv );
    LOAD_GLPROC( glProgramUniformMatrix3x4fv );
    LOAD_GLPROC( glProgramUniformMatrix4x3fv );
    LOAD_GLPROC( glValidateProgramPipeline );
    LOAD_GLPROC( glGetProgramPipelineInfoLog );
    LOAD_GLPROC( glBindImageTexture );
    LOAD_GLPROC( glGetBooleani_v );
    LOAD_GLPROC( glMemoryBarrier );
    LOAD_GLPROC( glMemoryBarrierByRegion );
    LOAD_GLPROC( glTexStorage2DMultisample );
    LOAD_GLPROC( glGetMultisamplefv );
    LOAD_GLPROC( glSampleMaski );
    LOAD_GLPROC( glGetTexLevelParameteriv );
    LOAD_GLPROC( glGetTexLevelParameterfv );
    LOAD_GLPROC( glBindVertexBuffer );
    LOAD_GLPROC( glVertexAttribFormat );
    LOAD_GLPROC( glVertexAttribIFormat );
    LOAD_GLPROC( glVertexAttribBinding );
    LOAD_GLPROC( glVertexBindingDivisor );
    return true;
}

#endif // /GL_ES_VERSION_3_1

#if GL_ES_VERSION_3_2

static bool DECL_LOADGLEXT_PROC(GLES_3_2)
{
    LOAD_GLPROC( glBlendBarrier );
    LOAD_GLPROC( glCopyImageSubData );
    LOAD_GLPROC( glDebugMessageControl );
    LOAD_GLPROC( glDebugMessageInsert );
    LOAD_GLPROC( glDebugMessageCallback );
    LOAD_GLPROC( glGetDebugMessageLog );
    LOAD_GLPROC( glPushDebugGroup );
    LOAD_GLPROC( glPopDebugGroup );
    LOAD_GLPROC( glObjectLabel );
    LOAD_GLPROC( glGetObjectLabel );
    LOAD_GLPROC( glObjectPtrLabel );
    LOAD_GLPROC( glGetObjectPtrLabel );
    LOAD_GLPROC( glGetPointerv );
    LOAD_GLPROC( glEnablei );
    LOAD_GLPROC( glDisablei );
    LOAD_GLPROC( glBlendEquationi );
    LOAD_GLPROC( glBlendEquationSeparatei );
    LOAD_GLPROC( glBlendFunci );
    LOAD_GLPROC( glBlendFuncSeparatei );
    LOAD_GLPROC( glColorMaski );
    LOAD_GLPROC( glIsEnabledi );
    LOAD_GLPROC( glDrawElementsBaseVertex );
    LOAD_GLPROC( glDrawRangeElementsBaseVertex );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertex );
    LOAD_GLPROC( glFramebufferTexture );
    LOAD_GLPROC( glPrimitiveBoundingBox );
    LOAD_GLPROC( glGetGraphicsResetStatus );
    LOAD_GLPROC( glReadnPixels );
    LOAD_GLPROC( glGetnUniformfv );
    LOAD_GLPROC( glGetnUniformiv );
    LOAD_GLPROC( glGetnUniformuiv );
    LOAD_GLPROC( glMinSampleShading );
    LOAD_GLPROC( glPatchParameteri );
    LOAD_GLPROC( glTexParameterIiv );
    LOAD_GLPROC( glTexParameterIuiv );
    LOAD_GLPROC( glGetTexParameterIiv );
    LOAD_GLPROC( glGetTexParameterIuiv );
    LOAD_GLPROC( glSamplerParameterIiv );
    LOAD_GLPROC( glSamplerParameterIuiv );
    LOAD_GLPROC( glGetSamplerParameterIiv );
    LOAD_GLPROC( glGetSamplerParameterIuiv );
    LOAD_GLPROC( glTexBuffer );
    LOAD_GLPROC( glTexBufferRange );
    LOAD_GLPROC( glTexStorage3DMultisample );
    return true;
}

#endif // /GL_ES_VERSION_3_2

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
        if (const char* extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)))
            extensions[extString] = false;
    }

    return extensions;
}

#endif // /__APPLE__

// Global member to store if the extension have already been loaded
static bool                     g_OpenGLESExtensionsLoaded = false;
static GLESExtensionMap         g_OpenGLESExtensionsMap;
static std::set<const char*>    g_supportedOpenGLESExtensions;
static std::set<const char*>    g_loadedOpenGLESExtensions;

static void EnableGLESExtension(GLExt ext, const char* name)
{
    RegisterExtension(ext);
    g_supportedOpenGLESExtensions.insert(name); //TODO: find better way to determine supported GLES extensions
    g_loadedOpenGLESExtensions.insert(name);
}

bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_OpenGLESExtensionsLoaded)
        return true;

    #define ENABLE_GLEXT(NAME) \
        EnableGLESExtension(GLExt::NAME, "GL_" #NAME)

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

    #ifndef __APPLE__

    /* Query supported OpenGL extension names */
    g_OpenGLESExtensionsMap = QuerySupportedOpenGLExtensions(isCoreProfile);

    auto LoadExtension = [abortOnFailure](const char* extName, const LoadGLExtensionProc& extLoadingProc) -> void
    {
        /* Try to load OpenGL extension */
        auto it = g_OpenGLESExtensionsMap.find(extName);
        if (it != g_OpenGLESExtensionsMap.end())
        {
            if (extLoadingProc(extName, abortOnFailure))
            {
                /* Enable extension in registry */
                //RegisterExtension(extensionID);
                it->second = true;
            }
        }
    };

    #define LOAD_GLEXT(NAME) \
        Load_GL_##NAME(#NAME, abortOnFailure)

    /* Load hardware buffer extensions */
    #if GL_ES_VERSION_3_1
    LOAD_GLEXT( GLES_3_1 );
    #endif

    #if GL_ES_VERSION_3_2
    LOAD_GLEXT( GLES_3_2 );
    #endif

    #undef LOAD_GLEXT

    #endif // /__APPLE__

    /* Cache supported and loaded extensions */
    g_OpenGLESExtensionsLoaded = true;

    for (const auto& it : g_OpenGLESExtensionsMap)
    {
        g_supportedOpenGLESExtensions.insert(it.first.c_str());
        if (it.second)
            g_loadedOpenGLESExtensions.insert(it.first.c_str());
    }

    return true;
}

bool AreOpenGLExtensionsLoaded()
{
    return g_OpenGLESExtensionsLoaded;
}

const std::set<const char*>& GetSupportedOpenGLExtensions()
{
    return g_supportedOpenGLESExtensions;
}

const std::set<const char*>& GetLoadedOpenGLExtensions()
{
    return g_loadedOpenGLESExtensions;
}


} // /namespace LLGL



// ================================================================================
