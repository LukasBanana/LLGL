/*
 * GLCompatExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Ext/GLExtensionLoader.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "../../../../Core/Exception.h"
#include "GLCompatExtensions.h"
#include <LLGL/Utils/ForRange.h>
#include <functional>
#include <string>
#include <map>


namespace LLGL
{


// OpenGL extension map type: Maps the extension name to boolean indicating whether or not the extension was loaded successully.
using GLExtensionMap = std::map<std::string, bool>;

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
    LLGL_TRAP("platform not supported for loading OpenGL extensions");
    #endif

    return (procAddr != nullptr);
}

static void ExtractExtensionsFromString(GLExtensionMap& extensions, const std::string& extString)
{
    size_t first = 0, last = 0;

    /* Find next extension name in string */
    while ( ( last = extString.find(' ', first) ) != std::string::npos )
    {
        /* Store current extension name in hash-map */
        auto name = extString.substr(first, last - first);
        extensions[name] = false;
        first = last + 1;
    }
}


#ifndef __APPLE__

using LoadGLExtensionProc = std::function<bool(const char* extName, bool abortOnFailure, bool usePlaceholder)>;

#define DECL_LOADGLEXT_PROC(EXTNAME) \
    Load_GL_ ## EXTNAME(const char* extName, bool abortOnFailure, bool usePlaceholder)

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#define LOAD_GLPROC(NAME)                                                           \
    if (usePlaceholder)                                                             \
    {                                                                               \
        NAME = Proxy_##NAME;                                                        \
    }                                                                               \
    else if (!LoadGLProc(NAME, #NAME))                                              \
    {                                                                               \
        if (abortOnFailure)                                                         \
            LLGL_TRAP("failed to load OpenGL procedure: %s [%s]", #NAME, extName);  \
        return false;                                                               \
    }

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

/* --- Core profile extensions extensions --- */

#if defined(GL_VERSION_3_1) && !defined(GL_GLEXT_PROTOTYPES)

static bool DECL_LOADGLEXT_PROC(ARB_compatibility)
{
    LOAD_GLPROC( glPrimitiveRestartIndex );
    return true;
}

#endif

/* --- Hardware buffer extensions --- */

static bool DECL_LOADGLEXT_PROC(ARB_vertex_buffer_object)
{
    LOAD_GLPROC( glGenBuffers           );
    LOAD_GLPROC( glDeleteBuffers        );
    LOAD_GLPROC( glBindBuffer           );
    LOAD_GLPROC( glIsBuffer             );
    LOAD_GLPROC( glBufferData           );
    LOAD_GLPROC( glBufferSubData        );
    LOAD_GLPROC( glGetBufferSubData     );
    LOAD_GLPROC( glMapBuffer            );
    LOAD_GLPROC( glUnmapBuffer          );
    LOAD_GLPROC( glGetBufferParameteriv );
    LOAD_GLPROC( glGetBufferPointerv    );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_map_buffer_range)
{
    LOAD_GLPROC( glMapBufferRange );
    LOAD_GLPROC( glFlushMappedBufferRange );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_vertex_shader)
{
    LOAD_GLPROC( glEnableVertexAttribArray  );
    LOAD_GLPROC( glDisableVertexAttribArray );
    LOAD_GLPROC( glVertexAttribPointer      );
    LOAD_GLPROC( glBindAttribLocation       );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_framebuffer_object)
{
    LOAD_GLPROC( glGenRenderbuffersEXT                    );
    LOAD_GLPROC( glDeleteRenderbuffersEXT                 );
    LOAD_GLPROC( glBindRenderbufferEXT                    );
    LOAD_GLPROC( glRenderbufferStorageEXT                 );
    LOAD_GLPROC( glRenderbufferStorageMultisampleEXT      );
    LOAD_GLPROC( glGetRenderbufferParameterivEXT          );
    LOAD_GLPROC( glGenFramebuffersEXT                     );
    LOAD_GLPROC( glDeleteFramebuffersEXT                  );
    LOAD_GLPROC( glBindFramebufferEXT                     );
    LOAD_GLPROC( glCheckFramebufferStatusEXT              );
    LOAD_GLPROC( glFramebufferTexture1DEXT                );
    LOAD_GLPROC( glFramebufferTexture2DEXT                );
    LOAD_GLPROC( glFramebufferTexture3DEXT                );
    LOAD_GLPROC( glFramebufferRenderbufferEXT             );
    LOAD_GLPROC( glGetFramebufferAttachmentParameterivEXT );
    LOAD_GLPROC( glBlitFramebufferEXT                     );
    LOAD_GLPROC( glGenerateMipmapEXT                      );
    return true;
}

/* --- Shader extensions --- */

static bool DECL_LOADGLEXT_PROC(ARB_shader_objects)
{
    LOAD_GLPROC( glCreateShader       );
    LOAD_GLPROC( glShaderSource       );
    LOAD_GLPROC( glCompileShader      );
    LOAD_GLPROC( glGetShaderiv        );
    LOAD_GLPROC( glGetShaderInfoLog   );
    LOAD_GLPROC( glGetShaderSource    );
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
    LOAD_GLPROC( glUniform1f          );
    LOAD_GLPROC( glUniform2f          );
    LOAD_GLPROC( glUniform3f          );
    LOAD_GLPROC( glUniform4f          );
    LOAD_GLPROC( glUniform1i          );
    LOAD_GLPROC( glUniform2i          );
    LOAD_GLPROC( glUniform3i          );
    LOAD_GLPROC( glUniform4i          );
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
    LOAD_GLPROC( glGetUniformiv       );
    LOAD_GLPROC( glGetUniformfv       );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_shader_objects_21)
{
    LOAD_GLPROC( glUniformMatrix2x3fv );
    LOAD_GLPROC( glUniformMatrix2x4fv );
    LOAD_GLPROC( glUniformMatrix3x2fv );
    LOAD_GLPROC( glUniformMatrix3x4fv );
    LOAD_GLPROC( glUniformMatrix4x2fv );
    LOAD_GLPROC( glUniformMatrix4x3fv );
    return true;
}

/* --- Texture extensions --- */

static bool DECL_LOADGLEXT_PROC(ARB_multitexture)
{
    LOAD_GLPROC( glActiveTexture );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_texture3D)
{
    LOAD_GLPROC( glTexImage3D    );
    LOAD_GLPROC( glTexSubImage3D );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_copy_texture)
{
    LOAD_GLPROC( glCopyTexSubImage3D );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_compression)
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

/* --- Other extensions --- */

static bool DECL_LOADGLEXT_PROC(ARB_occlusion_query)
{
    LOAD_GLPROC( glGenQueries        );
    LOAD_GLPROC( glDeleteQueries     );
    LOAD_GLPROC( glBeginQuery        );
    LOAD_GLPROC( glEndQuery          );
    LOAD_GLPROC( glGetQueryObjectiv  );
    LOAD_GLPROC( glGetQueryObjectuiv );
    return true;
}

static bool DECL_LOADGLEXT_PROC(NV_conditional_render)
{
    LOAD_GLPROC( glBeginConditionalRender );
    LOAD_GLPROC( glEndConditionalRender   );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_timer_query)
{
    LOAD_GLPROC( glQueryCounter        );
    LOAD_GLPROC( glGetQueryObjecti64v  );
    LOAD_GLPROC( glGetQueryObjectui64v );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_blend_minmax)
{
    LOAD_GLPROC( glBlendEquation );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_blend_color)
{
    LOAD_GLPROC( glBlendColor );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_blend_func_separate)
{
    LOAD_GLPROC( glBlendFuncSeparate );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_blend_equation_separate)
{
    LOAD_GLPROC( glBlendEquationSeparate );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_draw_buffers_blend)
{
    LOAD_GLPROC( glBlendEquationi         );
    LOAD_GLPROC( glBlendEquationSeparatei );
    LOAD_GLPROC( glBlendFunci             );
    LOAD_GLPROC( glBlendFuncSeparatei     );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_stencil_two_side)
{
    //correct extension ??? maybe "GL_ATI_separate_stencil"
    LOAD_GLPROC( glStencilFuncSeparate );
    LOAD_GLPROC( glStencilMaskSeparate );
    LOAD_GLPROC( glStencilOpSeparate   );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_draw_buffers)
{
    LOAD_GLPROC( glDrawBuffers );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_draw_buffers2)
{
    LOAD_GLPROC( glColorMaski    );
    LOAD_GLPROC( glGetBooleani_v );
    LOAD_GLPROC( glGetIntegeri_v );
    LOAD_GLPROC( glEnablei       );
    LOAD_GLPROC( glDisablei      );
    LOAD_GLPROC( glIsEnabledi    );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_transform_feedback)
{
    LOAD_GLPROC( glBindBufferRange             );
    LOAD_GLPROC( glBeginTransformFeedback      );
    LOAD_GLPROC( glEndTransformFeedback        );
    LOAD_GLPROC( glTransformFeedbackVaryings   );
    LOAD_GLPROC( glGetTransformFeedbackVarying );
    return true;
}

static bool DECL_LOADGLEXT_PROC(NV_transform_feedback)
{
    LOAD_GLPROC( glBindBufferRangeNV           );
    LOAD_GLPROC( glBeginTransformFeedbackNV    );
    LOAD_GLPROC( glEndTransformFeedbackNV      );
    LOAD_GLPROC( glTransformFeedbackVaryingsNV );
    LOAD_GLPROC( glGetVaryingLocationNV        );
    LOAD_GLPROC( glGetActiveVaryingNV          );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_sync)
{
    LOAD_GLPROC( glFenceSync      );
    LOAD_GLPROC( glIsSync         );
    LOAD_GLPROC( glDeleteSync     );
    LOAD_GLPROC( glClientWaitSync );
    LOAD_GLPROC( glWaitSync       );
    LOAD_GLPROC( glGetInteger64v  );
    LOAD_GLPROC( glGetSynciv      );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_internalformat_query)
{
    LOAD_GLPROC( glGetInternalformativ );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_internalformat_query2)
{
    LOAD_GLPROC( glGetInternalformati64v );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_ES2_compatibility)
{
    LOAD_GLPROC( glReleaseShaderCompiler    );
    LOAD_GLPROC( glShaderBinary             );
    LOAD_GLPROC( glGetShaderPrecisionFormat );
    LOAD_GLPROC( glDepthRangef              );
    LOAD_GLPROC( glClearDepthf              );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_buffer_storage)
{
    LOAD_GLPROC( glBufferStorage );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_copy_buffer)
{
    LOAD_GLPROC( glCopyBufferSubData );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_copy_image)
{
    LOAD_GLPROC( glCopyImageSubData );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_polygon_offset_clamp)
{
    LOAD_GLPROC( glPolygonOffsetClamp );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_clear_buffer_object)
{
    LOAD_GLPROC( glClearBufferData    );
    LOAD_GLPROC( glClearBufferSubData );
    return true;
}

#undef DECL_LOADGLEXT_PROC
#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC

#endif // /ifndef(__APPLE__)


/* --- Common extension loading functions --- */

static GLExtensionMap QuerySupportedOpenGLExtensions()
{
    GLExtensionMap extensions;

    /* Get complete extension string */
    if (const char* extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)))
        ExtractExtensionsFromString(extensions, extString);

    #if defined(_WIN32) && WGL_ARB_extensions_string

    /* Filter Win32 related extensions */
    if (wglGetExtensionsStringARB != nullptr || LoadGLProc(wglGetExtensionsStringARB, "wglGetExtensionsStringARB"))
    {
        if (const char* extString = wglGetExtensionsStringARB(::wglGetCurrentDC()))
            ExtractExtensionsFromString(extensions, extString);
    }

    #endif

    return extensions;
}

#ifndef __APPLE__

// Includes all GL extensions that are considered default for core profiles
static void IncludeDefaultCompatProfileExtensions(GLExtensionMap& extensions)
{
    static const char* compatProfileDefaultExtenions[] =
    {
        "GL_ARB_compatibility",
        "GL_ARB_multitexture",          // GL 1.2
        "GL_ARB_shader_objects",
        "GL_ARB_shader_objects_21",
        "GL_ARB_vertex_buffer_object",
        "GL_ARB_vertex_shader",
        "GL_EXT_blend_func_separate",   // GL 2.0
        "GL_EXT_copy_texture",
        "GL_EXT_gpu_shader4",           // GL 2.0
        "GL_EXT_stencil_two_side",      // GL 2.0
        "GL_EXT_texture3D",
    };
    for (const char* ext : compatProfileDefaultExtenions)
        extensions[ext] = false;
}

// Includes all GL extensions that are implied by other extensions
static void IncludeImpliedExtensions(GLExtensionMap& extensions)
{
    auto ImplyExtension = [&extensions](const char* originExtension, const std::initializer_list<const char*>& impliedExtensions)
    {
        if (extensions.find(originExtension) != extensions.end())
        {
            for (auto ext : impliedExtensions)
                extensions[ext] = false;
        }
    };
    ImplyExtension("GL_ARB_occlusion_query2", { "GL_ARB_occlusion_query" });
}

#endif // /__APPLE__

// Global member to store if the extension have already been loaded
static bool                     g_OpenGLExtensionsLoaded = false;
static GLExtensionMap           g_OpenGLExtensionsMap;
static std::set<const char*>    g_supportedOpenGLExtensions;
static std::set<const char*>    g_loadedOpenGLExtensions;

bool LoadSupportedOpenGLExtensions(bool /*isCoreProfile*/, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_OpenGLExtensionsLoaded)
        return true;

    /* Query supported OpenGL extension names */
    g_OpenGLExtensionsMap = QuerySupportedOpenGLExtensions();

    #ifdef __APPLE__

    /* Enable OpenGL extension support by host MacOS version */
    #define ENABLE_GLEXT(NAME) \
        RegisterExtension(GLExt::NAME)

    /* Enable basic GL functionality (such as glPrimitiveRestartIndex) */
    ENABLE_GLEXT( ARB_compatibility                );

    /* Enable hardware buffer extensions */
    ENABLE_GLEXT( ARB_vertex_buffer_object         );
    ENABLE_GLEXT( ARB_vertex_shader                );
    ENABLE_GLEXT( EXT_framebuffer_object           );
    ENABLE_GLEXT( ARB_map_buffer_range             );

    /* Enable shader extensions */
    ENABLE_GLEXT( ARB_shader_objects               );

    /* Enable texture extensions */
    ENABLE_GLEXT( ARB_multitexture                 );
    ENABLE_GLEXT( EXT_texture3D                    );
    ENABLE_GLEXT( EXT_copy_texture                 );
    ENABLE_GLEXT( ARB_clear_texture                );
    ENABLE_GLEXT( ARB_texture_compression          );
    ENABLE_GLEXT( ARB_texture_multisample          );

    /* Enable blending extensions */
    ENABLE_GLEXT( EXT_blend_minmax                 );
    ENABLE_GLEXT( EXT_blend_func_separate          );
    ENABLE_GLEXT( EXT_blend_equation_separate      );
    ENABLE_GLEXT( EXT_blend_color                  );
    ENABLE_GLEXT( ARB_draw_buffers_blend           );

    /* Enable misc extensions */
    ENABLE_GLEXT( ARB_occlusion_query              );
    ENABLE_GLEXT( NV_conditional_render            );
    ENABLE_GLEXT( ARB_timer_query                  );
    ENABLE_GLEXT( EXT_stencil_two_side             );
    ENABLE_GLEXT( ARB_draw_buffers                 );
    ENABLE_GLEXT( EXT_draw_buffers2                );
    ENABLE_GLEXT( EXT_transform_feedback           );
    ENABLE_GLEXT( ARB_sync                         );
    ENABLE_GLEXT( ARB_polygon_offset_clamp         );
    ENABLE_GLEXT( ARB_copy_buffer                  );
    ENABLE_GLEXT( ARB_draw_indirect                );
    ENABLE_GLEXT( ARB_multi_draw_indirect          );

    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_texture_cube_map             );

    #undef ENABLE_GLEXT

    #else // __APPLE__

    auto LoadExtension = [abortOnFailure](const char* extName, const LoadGLExtensionProc& extLoadingProc, GLExt extensionID) -> void
    {
        /* Try to load OpenGL extension */
        auto it = g_OpenGLExtensionsMap.find(extName);
        if (it != g_OpenGLExtensionsMap.end())
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

    auto EnableExtension = [&](const std::string& extName, GLExt extensionID) -> void
    {
        /* Try to enable OpenGL extension */
        if (g_OpenGLExtensionsMap.find(extName) != g_OpenGLExtensionsMap.end())
            RegisterExtension(extensionID);
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" #NAME, Load_GL_##NAME, GLExt::NAME)

    #define ENABLE_GLEXT(NAME) \
        EnableExtension("GL_" #NAME, GLExt::NAME)

    /* Add standard extensions */
    IncludeDefaultCompatProfileExtensions(g_OpenGLExtensionsMap);
    IncludeImpliedExtensions(g_OpenGLExtensionsMap);

    #if defined(GL_VERSION_3_1) && !defined(GL_GLEXT_PROTOTYPES)
    LOAD_GLEXT( ARB_compatibility                );
    #endif

    /* Load hardware buffer extensions */
    LOAD_GLEXT( ARB_vertex_buffer_object         ); // Always required for GL 2.x
    LOAD_GLEXT( ARB_vertex_shader                ); // Always required for GL 2.x
    LOAD_GLEXT( EXT_framebuffer_object           ); // Always required for GL 2.x
    LOAD_GLEXT( ARB_map_buffer_range             );

    /* Load shader extensions */
    LOAD_GLEXT( ARB_shader_objects               );
    LOAD_GLEXT( ARB_shader_objects_21            ); //TODO: load if GL version is high enough

    /* Load texture extensions */
    LOAD_GLEXT( ARB_multitexture                 );
    LOAD_GLEXT( EXT_texture3D                    );
    LOAD_GLEXT( EXT_copy_texture                 );
    LOAD_GLEXT( ARB_texture_compression          );

    /* Load blending extensions */
    LOAD_GLEXT( EXT_blend_minmax                 );
    LOAD_GLEXT( EXT_blend_func_separate          );
    LOAD_GLEXT( EXT_blend_equation_separate      );
    LOAD_GLEXT( EXT_blend_color                  );
    LOAD_GLEXT( ARB_draw_buffers_blend           );

    /* Load misc extensions */
    LOAD_GLEXT( ARB_occlusion_query              );
    LOAD_GLEXT( NV_conditional_render            );
    LOAD_GLEXT( ARB_timer_query                  );
    LOAD_GLEXT( EXT_stencil_two_side             );
    LOAD_GLEXT( ARB_draw_buffers                 );
    LOAD_GLEXT( EXT_draw_buffers2                );
    LOAD_GLEXT( EXT_transform_feedback           );
    LOAD_GLEXT( NV_transform_feedback            );
    LOAD_GLEXT( ARB_sync                         );
    LOAD_GLEXT( ARB_internalformat_query         );
    LOAD_GLEXT( ARB_internalformat_query2        );
    LOAD_GLEXT( ARB_ES2_compatibility            );
    LOAD_GLEXT( ARB_buffer_storage               );
    LOAD_GLEXT( ARB_copy_buffer                  );
    LOAD_GLEXT( ARB_copy_image                   );
    LOAD_GLEXT( ARB_polygon_offset_clamp         );
    LOAD_GLEXT( ARB_clear_buffer_object          );

    /* Enable extensions and ignore procedures */
    ENABLE_GLEXT( ARB_transform_feedback3 );

    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_texture_cube_map );

    #undef LOAD_GLEXT
    #undef ENABLE_GLEXT

    #endif // /__APPLE__

    /* Cache supported and loaded extensions */
    g_OpenGLExtensionsLoaded = true;

    for (const auto& it : g_OpenGLExtensionsMap)
    {
        g_supportedOpenGLExtensions.insert(it.first.c_str());
        if (it.second)
            g_loadedOpenGLExtensions.insert(it.first.c_str());
    }

    return true;
}

bool AreOpenGLExtensionsLoaded()
{
    return g_OpenGLExtensionsLoaded;
}

const std::set<const char*>& GetSupportedOpenGLExtensions()
{
    return g_supportedOpenGLExtensions;
}

const std::set<const char*>& GetLoadedOpenGLExtensions()
{
    return g_loadedOpenGLExtensions;
}


} // /namespace LLGL



// ================================================================================
