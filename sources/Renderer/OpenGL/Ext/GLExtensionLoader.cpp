/*
 * GLExtensionLoader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionLoader.h"
#include "GLExtensions.h"
#include "GLExtensionsNull.h"
#include <LLGL/Log.h>
#include <functional>


namespace LLGL
{


/* --- Internal functions --- */

template <typename T>
bool LoadGLProc(T& procAddr, const char* procName)
{
    /*
    Load OpenGL procedure address
    -> Make an exception with platform dependent code here, because we use a template function.
    */
    #if defined(_WIN32)
    procAddr = reinterpret_cast<T>(wglGetProcAddress(procName));
    #elif defined(__linux__)
    procAddr = reinterpret_cast<T>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(procName)));
    #else
    Log::StdErr() << "OS not supported for loading OpenGL extensions" << std::endl;
    return false;
    #endif
    
    /* Check for errors */
    if (!procAddr)
    {
        Log::StdErr() << "failed to load OpenGL procedure: " << procName << std::endl;
        return false;
    }
    
    return true;
}

static void ExtractExtensionsFromString(std::set<std::string>& extensions, const std::string& extString)
{
    size_t first = 0, last = 0;
    
    /* Find next extension name in string */
    while ( ( last = extString.find(' ', first) ) != std::string::npos )
    {
        /* Store current extension name in hash-map */
        auto name = extString.substr(first, last - first);
        extensions.insert(name);
        first = last + 1;
    }
}


#ifndef __APPLE__

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS

#define LOAD_GLPROC(NAME)               \
    if (usePlaceHolder)                 \
        NAME = Dummy_##NAME;            \
    else if (!LoadGLProc(NAME, #NAME))  \
        return false

#else

#define LOAD_GLPROC(NAME)           \
    if (!LoadGLProc(NAME, #NAME))   \
        return false

#endif

/* --- Common GL extensions --- */

bool LoadSwapIntervalProcs()
{
    #if defined(_WIN32)
    return LOAD_GLPROC_SIMPLE(wglSwapIntervalEXT);
    #elif defined(__linux__)
    return LOAD_GLPROC_SIMPLE(glXSwapIntervalSGI);
    #else
    return false;
    #endif
}

bool LoadPixelFormatProcs()
{
    #if defined(_WIN32)
    return LOAD_GLPROC_SIMPLE(wglChoosePixelFormatARB);
    #else
    return false;
    #endif
}

bool LoadCreateContextProcs()
{
    #if defined(_WIN32)
    return LOAD_GLPROC_SIMPLE(wglCreateContextAttribsARB);
    #else
    return false;
    #endif
}

/* --- Hardware buffer extensions --- */

static bool Load_GL_ARB_vertex_buffer_object(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenBuffers    );
    LOAD_GLPROC( glDeleteBuffers );
    LOAD_GLPROC( glBindBuffer    );
    LOAD_GLPROC( glBufferData    );
    LOAD_GLPROC( glBufferSubData );
    LOAD_GLPROC( glMapBuffer     );
    LOAD_GLPROC( glUnmapBuffer   );

    #if 1//TODO: which extension???
    LOAD_GLPROC( glEnableVertexAttribArray  );
    LOAD_GLPROC( glDisableVertexAttribArray );
    LOAD_GLPROC( glVertexAttribPointer      );
    LOAD_GLPROC( glBindAttribLocation       );
    #endif
    return true;
}

static bool Load_GL_ARB_vertex_array_object(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenVertexArrays    );
    LOAD_GLPROC( glDeleteVertexArrays );
    LOAD_GLPROC( glBindVertexArray    );
    return true;
}

static bool Load_GL_ARB_framebuffer_object(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenRenderbuffers                    );
    LOAD_GLPROC( glDeleteRenderbuffers                 );
    LOAD_GLPROC( glBindRenderbuffer                    );
    LOAD_GLPROC( glRenderbufferStorage                 );
    LOAD_GLPROC( glRenderbufferStorageMultisample      );
    LOAD_GLPROC( glGenFramebuffers                     );
    LOAD_GLPROC( glDeleteFramebuffers                  );
    LOAD_GLPROC( glBindFramebuffer                     );
    LOAD_GLPROC( glCheckFramebufferStatus              );
    LOAD_GLPROC( glFramebufferTexture                  );
    LOAD_GLPROC( glFramebufferTexture1D                );
    LOAD_GLPROC( glFramebufferTexture2D                );
    LOAD_GLPROC( glFramebufferTexture3D                );
    LOAD_GLPROC( glFramebufferTextureLayer             );
    LOAD_GLPROC( glFramebufferRenderbuffer             );
    LOAD_GLPROC( glGetFramebufferAttachmentParameteriv );
    LOAD_GLPROC( glBlitFramebuffer                     );
    LOAD_GLPROC( glGenerateMipmap                      );
    return true;
}

static bool Load_GL_ARB_uniform_buffer_object(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetUniformBlockIndex      );
    LOAD_GLPROC( glGetActiveUniformBlockiv   );
    LOAD_GLPROC( glGetActiveUniformBlockName );
    LOAD_GLPROC( glUniformBlockBinding       );
    LOAD_GLPROC( glBindBufferBase            );
    return true;
}

static bool Load_GL_ARB_shader_storage_buffer_object(bool usePlaceHolder)
{
    LOAD_GLPROC( glShaderStorageBlockBinding );
    return true;
}

/* --- Drawing extensions --- */

static bool Load_GL_ARB_draw_buffers(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawBuffers );
    return true;
}

static bool Load_GL_ARB_draw_instanced(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawArraysInstanced   );
    LOAD_GLPROC( glDrawElementsInstanced );
    return true;
}

static bool Load_GL_ARB_base_instance(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawArraysInstancedBaseInstance             );
    LOAD_GLPROC( glDrawElementsInstancedBaseInstance           );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertexBaseInstance );
    return true;
}

static bool Load_GL_ARB_draw_elements_base_vertex(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawElementsBaseVertex          );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertex );
    return true;
}

/* --- Shader extensions --- */

static bool Load_GL_ARB_shader_objects(bool usePlaceHolder)
{
    LOAD_GLPROC( glCreateShader       );
    LOAD_GLPROC( glShaderSource       );
    LOAD_GLPROC( glCompileShader      );
    LOAD_GLPROC( glGetShaderiv        );
    LOAD_GLPROC( glGetShaderInfoLog   );
    LOAD_GLPROC( glDeleteShader       );
    LOAD_GLPROC( glCreateProgram      );
    LOAD_GLPROC( glDeleteProgram      );
    LOAD_GLPROC( glAttachShader       );
    LOAD_GLPROC( glDetachShader       );
    LOAD_GLPROC( glLinkProgram        );
    LOAD_GLPROC( glValidateProgram    );
    LOAD_GLPROC( glGetProgramiv       );
    LOAD_GLPROC( glGetProgramInfoLog  );
    LOAD_GLPROC( glUseProgram         );
    LOAD_GLPROC( glGetActiveAttrib    );
    LOAD_GLPROC( glGetAttribLocation  );
    LOAD_GLPROC( glGetActiveUniform   );
    LOAD_GLPROC( glGetUniformLocation );
    LOAD_GLPROC( glGetAttachedShaders );
    LOAD_GLPROC( glUniform1fv         );
    LOAD_GLPROC( glUniform2fv         );
    LOAD_GLPROC( glUniform3fv         );
    LOAD_GLPROC( glUniform4fv         );
    LOAD_GLPROC( glUniform1iv         );
    LOAD_GLPROC( glUniform2iv         );
    LOAD_GLPROC( glUniform3iv         );
    LOAD_GLPROC( glUniform4iv         );
    LOAD_GLPROC( glUniformMatrix2fv   );
    LOAD_GLPROC( glUniformMatrix3fv   );
    LOAD_GLPROC( glUniformMatrix4fv   );
    return true;
}

static bool Load_GL_ARB_instanced_arrays(bool usePlaceHolder)
{
    LOAD_GLPROC( glVertexAttribDivisor );
    return true;
}

static bool Load_GL_ARB_tessellation_shader(bool usePlaceHolder)
{
    LOAD_GLPROC( glPatchParameteri  );
    LOAD_GLPROC( glPatchParameterfv );
    return true;
}

static bool Load_GL_ARB_compute_shader(bool usePlaceHolder)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}

static bool Load_GL_ARB_get_program_binary(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetProgramBinary  );
    LOAD_GLPROC( glProgramBinary     );
    LOAD_GLPROC( glProgramParameteri );
    return true;
}

static bool Load_GL_ARB_program_interface_query(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetProgramInterfaceiv           );
    LOAD_GLPROC( glGetProgramResourceIndex         );
    LOAD_GLPROC( glGetProgramResourceName          );
    LOAD_GLPROC( glGetProgramResourceiv            );
    LOAD_GLPROC( glGetProgramResourceLocation      );
    LOAD_GLPROC( glGetProgramResourceLocationIndex );
    return true;
}

static bool Load_GL_EXT_gpu_shader4(bool usePlaceHolder)
{
    LOAD_GLPROC( glVertexAttribIPointer );
    LOAD_GLPROC( glBindFragDataLocation );
    LOAD_GLPROC( glGetFragDataLocation  );
    return true;
}

/* --- Texture extensions --- */

static bool Load_GL_ARB_multitexture(bool usePlaceHolder)
{
    LOAD_GLPROC( glActiveTexture );
    return true;
}

static bool Load_GL_EXT_texture3D(bool usePlaceHolder)
{
    LOAD_GLPROC( glTexImage3D    );
    LOAD_GLPROC( glTexSubImage3D );
    return true;
}

static bool Load_GL_ARB_clear_texture(bool usePlaceHolder)
{
    LOAD_GLPROC( glClearTexImage    );
    LOAD_GLPROC( glClearTexSubImage );
    return true;
}

static bool Load_GL_ARB_texture_compression(bool usePlaceHolder)
{
    LOAD_GLPROC( glCompressedTexImage1D    );
    LOAD_GLPROC( glCompressedTexImage2D    );
    LOAD_GLPROC( glCompressedTexImage3D    );
    LOAD_GLPROC( glCompressedTexSubImage1D );
    LOAD_GLPROC( glCompressedTexSubImage2D );
    LOAD_GLPROC( glCompressedTexSubImage3D );
    LOAD_GLPROC( glGetCompressedTexImage   );
    return true;
}

static bool Load_GL_ARB_texture_multisample(bool usePlaceHolder)
{
    LOAD_GLPROC( glTexImage2DMultisample );
    LOAD_GLPROC( glTexImage3DMultisample );
    LOAD_GLPROC( glGetMultisamplefv      );
    LOAD_GLPROC( glSampleMaski           );
    return true;
}

static bool Load_GL_ARB_sampler_objects(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenSamplers        );
    LOAD_GLPROC( glDeleteSamplers     );
    LOAD_GLPROC( glBindSampler        );
    LOAD_GLPROC( glSamplerParameteri  );
    LOAD_GLPROC( glSamplerParameterf  );
    LOAD_GLPROC( glSamplerParameteriv );
    LOAD_GLPROC( glSamplerParameterfv );
    return true;
}

/* --- Other extensions --- */

static bool Load_GL_ARB_occlusion_query(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenQueries        );
    LOAD_GLPROC( glDeleteQueries     );
    LOAD_GLPROC( glBeginQuery        );
    LOAD_GLPROC( glEndQuery          );
    LOAD_GLPROC( glGetQueryObjectiv  );
    LOAD_GLPROC( glGetQueryObjectuiv );
    return true;
}

static bool Load_GL_NV_conditional_render(bool usePlaceHolder)
{
    LOAD_GLPROC( glBeginConditionalRender );
    LOAD_GLPROC( glEndConditionalRender   );
    return true;
}

static bool Load_GL_ARB_timer_query(bool usePlaceHolder)
{
    LOAD_GLPROC( glQueryCounter        );
    LOAD_GLPROC( glGetQueryObjecti64v  );
    LOAD_GLPROC( glGetQueryObjectui64v );
    return true;
}

static bool Load_GL_ARB_viewport_array(bool usePlaceHolder)
{
    LOAD_GLPROC( glViewportArrayv   );
    LOAD_GLPROC( glScissorArrayv    );
    LOAD_GLPROC( glDepthRangeArrayv );
    return true;
}

static bool Load_GL_EXT_blend_minmax(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendEquation );
    return true;
}

static bool Load_GL_EXT_blend_color(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendColor );
    return true;
}

static bool Load_GL_EXT_blend_func_separate(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendFuncSeparate );
    return true;
}

static bool Load_GL_EXT_blend_equation_separate(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendEquationSeparate );
    return true;
}

static bool Load_GL_ARB_draw_buffers_blend(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendEquationi         );
    LOAD_GLPROC( glBlendEquationSeparatei );
    LOAD_GLPROC( glBlendFunci             );
    LOAD_GLPROC( glBlendFuncSeparatei     );
    return true;
}

static bool Load_GL_ARB_multi_bind(bool usePlaceHolder)
{
    LOAD_GLPROC( glBindBuffersBase   );
    LOAD_GLPROC( glBindBuffersRange  );
    LOAD_GLPROC( glBindTextures      );
    LOAD_GLPROC( glBindSamplers      );
    LOAD_GLPROC( glBindImageTextures );
    LOAD_GLPROC( glBindVertexBuffers );
    return true;
}

static bool Load_GL_EXT_stencil_two_side(bool usePlaceHolder)
{
    //correct extension ??? maybe "GL_ATI_separate_stencil"
    LOAD_GLPROC( glStencilFuncSeparate );
    LOAD_GLPROC( glStencilMaskSeparate );
    LOAD_GLPROC( glStencilOpSeparate   );
    return true;
}

static bool Load_GL_KHR_debug(bool usePlaceHolder)
{
    LOAD_GLPROC( glDebugMessageCallback );
    return true;
}

static bool Load_GL_ARB_clip_control(bool usePlaceHolder)
{
    LOAD_GLPROC( glClipControl );
    return true;
}

static bool Load_GL_EXT_draw_buffers2(bool usePlaceHolder)
{
    LOAD_GLPROC( glColorMaski    );
    LOAD_GLPROC( glGetBooleani_v );
    LOAD_GLPROC( glGetIntegeri_v );
    LOAD_GLPROC( glEnablei       );
    LOAD_GLPROC( glDisablei      );
    LOAD_GLPROC( glIsEnabledi    );
    return true;
}

static bool Load_GL_EXT_transform_feedback(bool usePlaceHolder)
{
    LOAD_GLPROC( glBindBufferRange             );
    LOAD_GLPROC( glBeginTransformFeedback      );
    LOAD_GLPROC( glEndTransformFeedback        );
    LOAD_GLPROC( glTransformFeedbackVaryings   );
    LOAD_GLPROC( glGetTransformFeedbackVarying );
    return true;
}

static bool Load_GL_NV_transform_feedback(bool usePlaceHolder)
{
    LOAD_GLPROC( glBindBufferRangeNV           );
    LOAD_GLPROC( glBeginTransformFeedbackNV    );
    LOAD_GLPROC( glEndTransformFeedbackNV      );
    LOAD_GLPROC( glTransformFeedbackVaryingsNV );
    LOAD_GLPROC( glGetVaryingLocationNV        );
    LOAD_GLPROC( glGetActiveVaryingNV          );
    return true;
}

#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC
    
#endif


/* --- Common extension loading functions --- */

GLExtensionList QueryExtensions(bool coreProfile)
{
    GLExtensionList extensions;

    const char* extString = nullptr;
    
    /* Filter standard GL extensions */
    if (coreProfile)
    {
        #if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)
        
        #ifndef __APPLE__
        if (glGetStringi || LoadGLProc(glGetStringi, "glGetStringi"))
        #endif
        {
            /* Get number of extensions */
            GLint numExtensions = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
            
            for (int i = 0; i < numExtensions; ++i)
            {
                /* Get current extension string */
                extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
                if (extString)
                    extensions.insert(std::string(extString));
            }
        }
        
        #endif
    }
    else
    {
        /* Get complete extension string */
        extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extString)
            ExtractExtensionsFromString(extensions, extString);
    }

    #if defined(_WIN32) && defined(WGL_ARB_extensions_string)

    /* Filter Win32 related extensions */
    if (wglGetExtensionsStringARB || LoadGLProc(wglGetExtensionsStringARB, "wglGetExtensionsStringARB"))
    {
        extString = wglGetExtensionsStringARB(wglGetCurrentDC());
        if (extString)
            ExtractExtensionsFromString(extensions, extString);
    }
    
    #endif

    return extensions;
}

// Global member to store if the extension have already been loaded
static bool g_extAlreadyLoaded = false;

void LoadAllExtensions(GLExtensionList& extensions)
{
    /* Only load GL extensions once */
    if (g_extAlreadyLoaded)
        return;
    
    #ifdef __APPLE__
    
    /* Enable OpenGL extension support by host MacOS version */
    #define ENABLE_GLEXT(NAME) \
        EnableExtensionSupport(GLExt::NAME)
    
    /* Enable hardware buffer extensions */
    ENABLE_GLEXT( ARB_vertex_buffer_object         );
    ENABLE_GLEXT( ARB_vertex_array_object          );
    ENABLE_GLEXT( ARB_framebuffer_object           );
    ENABLE_GLEXT( ARB_uniform_buffer_object        );
    ENABLE_GLEXT( ARB_shader_storage_buffer_object );
    
    /* Enable drawing extensions */
    ENABLE_GLEXT( ARB_draw_buffers                 );
    ENABLE_GLEXT( ARB_draw_instanced               );
    ENABLE_GLEXT( ARB_base_instance                );
    ENABLE_GLEXT( ARB_draw_elements_base_vertex    );
    
    /* Enable shader extensions */
    ENABLE_GLEXT( ARB_shader_objects               );
    ENABLE_GLEXT( ARB_instanced_arrays             );
    ENABLE_GLEXT( ARB_tessellation_shader          );
    ENABLE_GLEXT( ARB_compute_shader               );
    ENABLE_GLEXT( ARB_get_program_binary           );
    ENABLE_GLEXT( ARB_program_interface_query      );
    ENABLE_GLEXT( EXT_gpu_shader4                  );
    
    /* Enable texture extensions */
    ENABLE_GLEXT( ARB_multitexture                 );
    ENABLE_GLEXT( EXT_texture3D                    );
    ENABLE_GLEXT( ARB_clear_texture                );
    ENABLE_GLEXT( ARB_texture_compression          );
    ENABLE_GLEXT( ARB_texture_multisample          );
    ENABLE_GLEXT( ARB_sampler_objects              );
    
    /* Enable blending extensions */
    ENABLE_GLEXT( EXT_blend_minmax                 );
    ENABLE_GLEXT( EXT_blend_func_separate          );
    ENABLE_GLEXT( EXT_blend_equation_separate      );
    ENABLE_GLEXT( EXT_blend_color                  );
    ENABLE_GLEXT( ARB_draw_buffers_blend           );
    
    /* Enable misc extensions */
    ENABLE_GLEXT( ARB_viewport_array               );
    ENABLE_GLEXT( ARB_occlusion_query              );
    ENABLE_GLEXT( NV_conditional_render            );
    ENABLE_GLEXT( ARB_timer_query                  );
    ENABLE_GLEXT( ARB_multi_bind                   );
    ENABLE_GLEXT( EXT_stencil_two_side             );
    ENABLE_GLEXT( KHR_debug                        );
    ENABLE_GLEXT( ARB_clip_control                 );
    ENABLE_GLEXT( EXT_draw_buffers2                );
    ENABLE_GLEXT( EXT_transform_feedback           );
    ENABLE_GLEXT( NV_transform_feedback            );
    
    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_texture_cube_map             );
    ENABLE_GLEXT( EXT_texture_array                );
    ENABLE_GLEXT( ARB_texture_cube_map_array       );
    ENABLE_GLEXT( ARB_geometry_shader4             );
    ENABLE_GLEXT( NV_conservative_raster           );
    ENABLE_GLEXT( INTEL_conservative_rasterization );
    
    #undef ENABLE_GLEXT
    
    #else
    
    auto LoadExtension = [&](const std::string& extName, const std::function<bool(bool)>& extLoadingProc, GLExt viewerExt) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extensions.find(extName);
        if (it != extensions.end() && !extLoadingProc(false))
        {
            Log::StdErr() << "failed to load OpenGL extension: " << extName << std::endl;
            extensions.erase(it);
        }
        #ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS
        else if (it == extensions.end())
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(true);
        }
        #endif
        else if (it != extensions.end())
        {
            /* Enable extension in viewer */
            EnableExtensionSupport(viewerExt);
        }
    };

    auto EnableExtension = [&](const std::string& extName, GLExt viewerExt) -> void
    {
        /* Try to enable OpenGL extension */
        if (extensions.find(extName) != extensions.end())
            EnableExtensionSupport(viewerExt);
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" + std::string(#NAME), Load_GL_##NAME, GLExt::NAME)

    #define ENABLE_GLEXT(NAME) \
        EnableExtension("GL_" + std::string(#NAME), GLExt::NAME)

    /* Load hardware buffer extensions */
    LOAD_GLEXT( ARB_vertex_buffer_object         );
    LOAD_GLEXT( ARB_vertex_array_object          );
    LOAD_GLEXT( ARB_framebuffer_object           );
    LOAD_GLEXT( ARB_uniform_buffer_object        );
    LOAD_GLEXT( ARB_shader_storage_buffer_object );

    /* Load drawing extensions */
    LOAD_GLEXT( ARB_draw_buffers                 );
    LOAD_GLEXT( ARB_draw_instanced               );
    LOAD_GLEXT( ARB_base_instance                );
    LOAD_GLEXT( ARB_draw_elements_base_vertex    );

    /* Load shader extensions */
    LOAD_GLEXT( ARB_shader_objects               );
    LOAD_GLEXT( ARB_instanced_arrays             );
    LOAD_GLEXT( ARB_tessellation_shader          );
    LOAD_GLEXT( ARB_compute_shader               );
    LOAD_GLEXT( ARB_get_program_binary           );
    LOAD_GLEXT( ARB_program_interface_query      );
    LOAD_GLEXT( EXT_gpu_shader4                  );

    /* Load texture extensions */
    LOAD_GLEXT( ARB_multitexture                 );
    LOAD_GLEXT( EXT_texture3D                    );
    LOAD_GLEXT( ARB_clear_texture                );
    LOAD_GLEXT( ARB_texture_compression          );
    LOAD_GLEXT( ARB_texture_multisample          );
    LOAD_GLEXT( ARB_sampler_objects              );

    /* Load blending extensions */
    LOAD_GLEXT( EXT_blend_minmax                 );
    LOAD_GLEXT( EXT_blend_func_separate          );
    LOAD_GLEXT( EXT_blend_equation_separate      );
    LOAD_GLEXT( EXT_blend_color                  );
    LOAD_GLEXT( ARB_draw_buffers_blend           );

    /* Load misc extensions */
    LOAD_GLEXT( ARB_viewport_array               );
    LOAD_GLEXT( ARB_occlusion_query              );
    LOAD_GLEXT( NV_conditional_render            );
    LOAD_GLEXT( ARB_timer_query                  );
    LOAD_GLEXT( ARB_multi_bind                   );
    LOAD_GLEXT( EXT_stencil_two_side             );
    LOAD_GLEXT( KHR_debug                        );
    LOAD_GLEXT( ARB_clip_control                 );
    LOAD_GLEXT( EXT_draw_buffers2                );
    LOAD_GLEXT( EXT_transform_feedback           );
    LOAD_GLEXT( NV_transform_feedback            );

    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_texture_cube_map             );
    ENABLE_GLEXT( EXT_texture_array                );
    ENABLE_GLEXT( ARB_texture_cube_map_array       );
    ENABLE_GLEXT( ARB_geometry_shader4             );
    ENABLE_GLEXT( NV_conservative_raster           );
    ENABLE_GLEXT( INTEL_conservative_rasterization );

    #undef LOAD_GLEXT
    #undef ENABLE_GLEXT
    
    #endif
    
    g_extAlreadyLoaded = true;
}

bool AreExtensionsLoaded()
{
    return g_extAlreadyLoaded;
}


} // /namespace LLGL



// ================================================================================
