/*
 * GLCoreExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Ext/GLExtensionLoader.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "../../../../Core/Exception.h"
#include "GLCoreExtensions.h"
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
    if (glXSwapIntervalSGI  != nullptr ||
        glXSwapIntervalMESA != nullptr ||
        glXSwapIntervalEXT  != nullptr)
    {
        /* Extension already loaded */
        return true;
    }
    const bool hasSwapIntervalSGI   = LOAD_GLPROC_SIMPLE(glXSwapIntervalSGI);
    const bool hasSwapIntervalMESA  = LOAD_GLPROC_SIMPLE(glXSwapIntervalMESA);
    const bool hasSwapIntervalEXT   = LOAD_GLPROC_SIMPLE(glXSwapIntervalEXT);
    return (hasSwapIntervalSGI || hasSwapIntervalMESA || hasSwapIntervalEXT);
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

static bool DECL_LOADGLEXT_PROC(ARB_vertex_array_object)
{
    LOAD_GLPROC( glGenVertexArrays    );
    LOAD_GLPROC( glDeleteVertexArrays );
    LOAD_GLPROC( glBindVertexArray    );
    LOAD_GLPROC( glIsVertexArray      );
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

static bool DECL_LOADGLEXT_PROC(ARB_framebuffer_object)
{
    LOAD_GLPROC( glGenRenderbuffers                    );
    LOAD_GLPROC( glDeleteRenderbuffers                 );
    LOAD_GLPROC( glBindRenderbuffer                    );
    LOAD_GLPROC( glRenderbufferStorage                 );
    LOAD_GLPROC( glRenderbufferStorageMultisample      );
    LOAD_GLPROC( glGetRenderbufferParameteriv          );
    LOAD_GLPROC( glGenFramebuffers                     );
    LOAD_GLPROC( glDeleteFramebuffers                  );
    LOAD_GLPROC( glBindFramebuffer                     );
    LOAD_GLPROC( glCheckFramebufferStatus              );
    //LOAD_GLPROC( glFramebufferTexture                  ); // <--- other extension! (but which one???)
    LOAD_GLPROC( glFramebufferTexture1D                );
    LOAD_GLPROC( glFramebufferTexture2D                );
    LOAD_GLPROC( glFramebufferTexture3D                );
    LOAD_GLPROC( glFramebufferTextureLayer             );
    LOAD_GLPROC( glFramebufferRenderbuffer             );
    LOAD_GLPROC( glGetFramebufferAttachmentParameteriv );
    LOAD_GLPROC( glBlitFramebuffer                     );
    LOAD_GLPROC( glGenerateMipmap                      );
    #if 1//TODO: other extension! (but which one???)
    LOAD_GLPROC( glClearBufferiv                       );
    LOAD_GLPROC( glClearBufferuiv                      );
    LOAD_GLPROC( glClearBufferfv                       );
    LOAD_GLPROC( glClearBufferfi                       );
    #endif
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_uniform_buffer_object)
{
    LOAD_GLPROC( glGetUniformBlockIndex      );
    LOAD_GLPROC( glGetActiveUniformBlockiv   );
    LOAD_GLPROC( glGetActiveUniformBlockName );
    LOAD_GLPROC( glUniformBlockBinding       );
    LOAD_GLPROC( glBindBufferBase            );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_shader_storage_buffer_object)
{
    LOAD_GLPROC( glShaderStorageBlockBinding );
    return true;
}

/* --- Drawing extensions --- */

static bool DECL_LOADGLEXT_PROC(ARB_draw_instanced)
{
    LOAD_GLPROC( glDrawArraysInstanced   );
    LOAD_GLPROC( glDrawElementsInstanced );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_base_instance)
{
    LOAD_GLPROC( glDrawArraysInstancedBaseInstance             );
    LOAD_GLPROC( glDrawElementsInstancedBaseInstance           );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertexBaseInstance );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_draw_elements_base_vertex)
{
    LOAD_GLPROC( glDrawElementsBaseVertex          );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertex );
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

static bool DECL_LOADGLEXT_PROC(ARB_shader_objects_30)
{
    LOAD_GLPROC( glUniform1uiv );
    LOAD_GLPROC( glUniform2uiv );
    LOAD_GLPROC( glUniform3uiv );
    LOAD_GLPROC( glUniform4uiv );
    return true;
}

//TODO: no determined yet when to load this extension
static bool DECL_LOADGLEXT_PROC(ARB_shader_objects_40)
{
    LOAD_GLPROC( glUniform1dv         );
    LOAD_GLPROC( glUniform2dv         );
    LOAD_GLPROC( glUniform3dv         );
    LOAD_GLPROC( glUniform4dv         );
    LOAD_GLPROC( glUniformMatrix2dv   );
    LOAD_GLPROC( glUniformMatrix3dv   );
    LOAD_GLPROC( glUniformMatrix4dv   );
    LOAD_GLPROC( glUniformMatrix2x3dv );
    LOAD_GLPROC( glUniformMatrix2x4dv );
    LOAD_GLPROC( glUniformMatrix3x2dv );
    LOAD_GLPROC( glUniformMatrix3x4dv );
    LOAD_GLPROC( glUniformMatrix4x2dv );
    LOAD_GLPROC( glUniformMatrix4x3dv );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_instanced_arrays)
{
    LOAD_GLPROC( glVertexAttribDivisor );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_tessellation_shader)
{
    LOAD_GLPROC( glPatchParameteri  );
    LOAD_GLPROC( glPatchParameterfv );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_compute_shader)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_get_program_binary)
{
    LOAD_GLPROC( glGetProgramBinary  );
    LOAD_GLPROC( glProgramBinary     );
    LOAD_GLPROC( glProgramParameteri ); // Duplicate in GL_ARB_separate_shader_objects
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_program_interface_query)
{
    LOAD_GLPROC( glGetProgramInterfaceiv           );
    LOAD_GLPROC( glGetProgramResourceIndex         );
    LOAD_GLPROC( glGetProgramResourceName          );
    LOAD_GLPROC( glGetProgramResourceiv            );
    LOAD_GLPROC( glGetProgramResourceLocation      );
    LOAD_GLPROC( glGetProgramResourceLocationIndex );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_separate_shader_objects)
{
    LOAD_GLPROC( glUseProgramStages          );
    LOAD_GLPROC( glActiveShaderProgram       );
    LOAD_GLPROC( glCreateShaderProgramv      );
    LOAD_GLPROC( glBindProgramPipeline       );
    LOAD_GLPROC( glDeleteProgramPipelines    );
    LOAD_GLPROC( glGenProgramPipelines       );
    LOAD_GLPROC( glIsProgramPipeline         );
    LOAD_GLPROC( glProgramParameteri         ); // Duplicate in GL_ARB_get_program_binary
    LOAD_GLPROC( glGetProgramPipelineiv      );
    LOAD_GLPROC( glProgramUniform1i          );
    LOAD_GLPROC( glProgramUniform2i          );
    LOAD_GLPROC( glProgramUniform3i          );
    LOAD_GLPROC( glProgramUniform4i          );
    LOAD_GLPROC( glProgramUniform1ui         );
    LOAD_GLPROC( glProgramUniform2ui         );
    LOAD_GLPROC( glProgramUniform3ui         );
    LOAD_GLPROC( glProgramUniform4ui         );
    LOAD_GLPROC( glProgramUniform1f          );
    LOAD_GLPROC( glProgramUniform2f          );
    LOAD_GLPROC( glProgramUniform3f          );
    LOAD_GLPROC( glProgramUniform4f          );
    LOAD_GLPROC( glProgramUniform1d          );
    LOAD_GLPROC( glProgramUniform2d          );
    LOAD_GLPROC( glProgramUniform3d          );
    LOAD_GLPROC( glProgramUniform4d          );
    LOAD_GLPROC( glProgramUniform1iv         );
    LOAD_GLPROC( glProgramUniform2iv         );
    LOAD_GLPROC( glProgramUniform3iv         );
    LOAD_GLPROC( glProgramUniform4iv         );
    LOAD_GLPROC( glProgramUniform1uiv        );
    LOAD_GLPROC( glProgramUniform2uiv        );
    LOAD_GLPROC( glProgramUniform3uiv        );
    LOAD_GLPROC( glProgramUniform4uiv        );
    LOAD_GLPROC( glProgramUniform1fv         );
    LOAD_GLPROC( glProgramUniform2fv         );
    LOAD_GLPROC( glProgramUniform3fv         );
    LOAD_GLPROC( glProgramUniform4fv         );
    LOAD_GLPROC( glProgramUniform1dv         );
    LOAD_GLPROC( glProgramUniform2dv         );
    LOAD_GLPROC( glProgramUniform3dv         );
    LOAD_GLPROC( glProgramUniform4dv         );
    LOAD_GLPROC( glProgramUniformMatrix2fv   );
    LOAD_GLPROC( glProgramUniformMatrix3fv   );
    LOAD_GLPROC( glProgramUniformMatrix4fv   );
    LOAD_GLPROC( glProgramUniformMatrix2dv   );
    LOAD_GLPROC( glProgramUniformMatrix3dv   );
    LOAD_GLPROC( glProgramUniformMatrix4dv   );
    LOAD_GLPROC( glProgramUniformMatrix2x3fv );
    LOAD_GLPROC( glProgramUniformMatrix3x2fv );
    LOAD_GLPROC( glProgramUniformMatrix2x4fv );
    LOAD_GLPROC( glProgramUniformMatrix4x2fv );
    LOAD_GLPROC( glProgramUniformMatrix3x4fv );
    LOAD_GLPROC( glProgramUniformMatrix4x3fv );
    LOAD_GLPROC( glProgramUniformMatrix2x3dv );
    LOAD_GLPROC( glProgramUniformMatrix3x2dv );
    LOAD_GLPROC( glProgramUniformMatrix2x4dv );
    LOAD_GLPROC( glProgramUniformMatrix4x2dv );
    LOAD_GLPROC( glProgramUniformMatrix3x4dv );
    LOAD_GLPROC( glProgramUniformMatrix4x3dv );
    LOAD_GLPROC( glValidateProgramPipeline   );
    LOAD_GLPROC( glGetProgramPipelineInfoLog );
    return true;
}

static bool DECL_LOADGLEXT_PROC(EXT_gpu_shader4)
{
    LOAD_GLPROC( glVertexAttribIPointer );
    LOAD_GLPROC( glBindFragDataLocation );
    LOAD_GLPROC( glGetFragDataLocation  );
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

static bool DECL_LOADGLEXT_PROC(ARB_clear_texture)
{
    LOAD_GLPROC( glClearTexImage    );
    LOAD_GLPROC( glClearTexSubImage );
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

static bool DECL_LOADGLEXT_PROC(ARB_texture_multisample)
{
    LOAD_GLPROC( glTexImage2DMultisample );
    LOAD_GLPROC( glTexImage3DMultisample );
    LOAD_GLPROC( glGetMultisamplefv      );
    LOAD_GLPROC( glSampleMaski           );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_view)
{
    LOAD_GLPROC( glTextureView );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_sampler_objects)
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

static bool DECL_LOADGLEXT_PROC(ARB_viewport_array)
{
    LOAD_GLPROC( glViewportArrayv   );
    LOAD_GLPROC( glScissorArrayv    );
    LOAD_GLPROC( glDepthRangeArrayv );
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

static bool DECL_LOADGLEXT_PROC(ARB_multi_bind)
{
    LOAD_GLPROC( glBindBuffersBase   );
    LOAD_GLPROC( glBindBuffersRange  );
    LOAD_GLPROC( glBindTextures      );
    LOAD_GLPROC( glBindSamplers      );
    LOAD_GLPROC( glBindImageTextures );
    LOAD_GLPROC( glBindVertexBuffers );
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

static bool DECL_LOADGLEXT_PROC(KHR_debug)
{
    LOAD_GLPROC( glDebugMessageControl  );
    LOAD_GLPROC( glDebugMessageInsert   );
    LOAD_GLPROC( glDebugMessageCallback );
    LOAD_GLPROC( glGetDebugMessageLog   );
  //LOAD_GLPROC( glGetPointerv          );
    LOAD_GLPROC( glPushDebugGroup       );
    LOAD_GLPROC( glPopDebugGroup        );
    LOAD_GLPROC( glObjectLabel          );
    LOAD_GLPROC( glGetObjectLabel       );
    LOAD_GLPROC( glObjectPtrLabel       );
    LOAD_GLPROC( glGetObjectPtrLabel    );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_clip_control)
{
    LOAD_GLPROC( glClipControl );
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

static bool DECL_LOADGLEXT_PROC(ARB_transform_feedback2)
{
    LOAD_GLPROC( glBindTransformFeedback    );
    LOAD_GLPROC( glDeleteTransformFeedbacks );
    LOAD_GLPROC( glGenTransformFeedbacks    );
    LOAD_GLPROC( glIsTransformFeedback      );
    LOAD_GLPROC( glPauseTransformFeedback   );
    LOAD_GLPROC( glResumeTransformFeedback  );
    LOAD_GLPROC( glDrawTransformFeedback    );
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

static bool DECL_LOADGLEXT_PROC(ARB_gl_spirv)
{
    LOAD_GLPROC( glSpecializeShader );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_storage)
{
    LOAD_GLPROC( glTexStorage1D );
    LOAD_GLPROC( glTexStorage2D );
    LOAD_GLPROC( glTexStorage3D );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_storage_multisample)
{
    LOAD_GLPROC( glTexStorage2DMultisample );
    LOAD_GLPROC( glTexStorage3DMultisample );
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

static bool DECL_LOADGLEXT_PROC(ARB_shader_image_load_store)
{
    LOAD_GLPROC( glBindImageTexture );
    LOAD_GLPROC( glMemoryBarrier    );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_framebuffer_no_attachments)
{
    LOAD_GLPROC( glFramebufferParameteri     );
    LOAD_GLPROC( glGetFramebufferParameteriv );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_clear_buffer_object)
{
    LOAD_GLPROC( glClearBufferData    );
    LOAD_GLPROC( glClearBufferSubData );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_draw_indirect)
{
    LOAD_GLPROC( glDrawArraysIndirect   );
    LOAD_GLPROC( glDrawElementsIndirect );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_multi_draw_indirect)
{
    LOAD_GLPROC( glMultiDrawArraysIndirect   );
    LOAD_GLPROC( glMultiDrawElementsIndirect );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_get_texture_sub_image)
{
    LOAD_GLPROC( glGetTextureSubImage           );
    LOAD_GLPROC( glGetCompressedTextureSubImage );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_buffer_object)
{
    LOAD_GLPROC( glTexBuffer );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_texture_buffer_range)
{
    LOAD_GLPROC( glTexBufferRange );
    return true;
}

static bool DECL_LOADGLEXT_PROC(ARB_direct_state_access)
{
    LOAD_GLPROC( glCreateTransformFeedbacks                 );
    LOAD_GLPROC( glTransformFeedbackBufferBase              );
    LOAD_GLPROC( glTransformFeedbackBufferRange             );
    LOAD_GLPROC( glGetTransformFeedbackiv                   );
    LOAD_GLPROC( glGetTransformFeedbacki_v                  );
    LOAD_GLPROC( glGetTransformFeedbacki64_v                );
    LOAD_GLPROC( glCreateBuffers                            );
    LOAD_GLPROC( glNamedBufferStorage                       );
    LOAD_GLPROC( glNamedBufferData                          );
    LOAD_GLPROC( glNamedBufferSubData                       );
    LOAD_GLPROC( glCopyNamedBufferSubData                   );
    LOAD_GLPROC( glClearNamedBufferData                     );
    LOAD_GLPROC( glClearNamedBufferSubData                  );
    LOAD_GLPROC( glMapNamedBuffer                           );
    LOAD_GLPROC( glMapNamedBufferRange                      );
    LOAD_GLPROC( glUnmapNamedBuffer                         );
    LOAD_GLPROC( glFlushMappedNamedBufferRange              );
    LOAD_GLPROC( glGetNamedBufferParameteriv                );
    LOAD_GLPROC( glGetNamedBufferParameteri64v              );
    LOAD_GLPROC( glGetNamedBufferPointerv                   );
    LOAD_GLPROC( glGetNamedBufferSubData                    );
    LOAD_GLPROC( glCreateFramebuffers                       );
    LOAD_GLPROC( glNamedFramebufferRenderbuffer             );
    LOAD_GLPROC( glNamedFramebufferParameteri               );
    LOAD_GLPROC( glNamedFramebufferTexture                  );
    LOAD_GLPROC( glNamedFramebufferTextureLayer             );
    LOAD_GLPROC( glNamedFramebufferDrawBuffer               );
    LOAD_GLPROC( glNamedFramebufferDrawBuffers              );
    LOAD_GLPROC( glNamedFramebufferReadBuffer               );
    LOAD_GLPROC( glInvalidateNamedFramebufferData           );
    LOAD_GLPROC( glInvalidateNamedFramebufferSubData        );
    LOAD_GLPROC( glClearNamedFramebufferiv                  );
    LOAD_GLPROC( glClearNamedFramebufferuiv                 );
    LOAD_GLPROC( glClearNamedFramebufferfv                  );
    LOAD_GLPROC( glClearNamedFramebufferfi                  );
    LOAD_GLPROC( glBlitNamedFramebuffer                     );
    LOAD_GLPROC( glCheckNamedFramebufferStatus              );
    LOAD_GLPROC( glGetNamedFramebufferParameteriv           );
    LOAD_GLPROC( glGetNamedFramebufferAttachmentParameteriv );
    LOAD_GLPROC( glCreateRenderbuffers                      );
    LOAD_GLPROC( glNamedRenderbufferStorage                 );
    LOAD_GLPROC( glNamedRenderbufferStorageMultisample      );
    LOAD_GLPROC( glGetNamedRenderbufferParameteriv          );
    LOAD_GLPROC( glCreateTextures                           );
    LOAD_GLPROC( glTextureBuffer                            );
    LOAD_GLPROC( glTextureBufferRange                       );
    LOAD_GLPROC( glTextureStorage1D                         );
    LOAD_GLPROC( glTextureStorage2D                         );
    LOAD_GLPROC( glTextureStorage3D                         );
    LOAD_GLPROC( glTextureStorage2DMultisample              );
    LOAD_GLPROC( glTextureStorage3DMultisample              );
    LOAD_GLPROC( glTextureSubImage1D                        );
    LOAD_GLPROC( glTextureSubImage2D                        );
    LOAD_GLPROC( glTextureSubImage3D                        );
    LOAD_GLPROC( glCompressedTextureSubImage1D              );
    LOAD_GLPROC( glCompressedTextureSubImage2D              );
    LOAD_GLPROC( glCompressedTextureSubImage3D              );
    LOAD_GLPROC( glCopyTextureSubImage1D                    );
    LOAD_GLPROC( glCopyTextureSubImage2D                    );
    LOAD_GLPROC( glCopyTextureSubImage3D                    );
    LOAD_GLPROC( glTextureParameterf                        );
    LOAD_GLPROC( glTextureParameterfv                       );
    LOAD_GLPROC( glTextureParameteri                        );
    LOAD_GLPROC( glTextureParameterIiv                      );
    LOAD_GLPROC( glTextureParameterIuiv                     );
    LOAD_GLPROC( glTextureParameteriv                       );
    LOAD_GLPROC( glGenerateTextureMipmap                    );
    LOAD_GLPROC( glBindTextureUnit                          );
    LOAD_GLPROC( glGetTextureImage                          );
    LOAD_GLPROC( glGetCompressedTextureImage                );
    LOAD_GLPROC( glGetTextureLevelParameterfv               );
    LOAD_GLPROC( glGetTextureLevelParameteriv               );
    LOAD_GLPROC( glGetTextureParameterfv                    );
    LOAD_GLPROC( glGetTextureParameterIiv                   );
    LOAD_GLPROC( glGetTextureParameterIuiv                  );
    LOAD_GLPROC( glGetTextureParameteriv                    );
    LOAD_GLPROC( glCreateVertexArrays                       );
    LOAD_GLPROC( glDisableVertexArrayAttrib                 );
    LOAD_GLPROC( glEnableVertexArrayAttrib                  );
    LOAD_GLPROC( glVertexArrayElementBuffer                 );
    LOAD_GLPROC( glVertexArrayVertexBuffer                  );
    LOAD_GLPROC( glVertexArrayVertexBuffers                 );
    LOAD_GLPROC( glVertexArrayAttribFormat                  );
    LOAD_GLPROC( glVertexArrayAttribIFormat                 );
    LOAD_GLPROC( glVertexArrayAttribLFormat                 );
    LOAD_GLPROC( glVertexArrayAttribBinding                 );
    LOAD_GLPROC( glVertexArrayBindingDivisor                );
    LOAD_GLPROC( glGetVertexArrayiv                         );
    LOAD_GLPROC( glGetVertexArrayIndexediv                  );
    LOAD_GLPROC( glGetVertexArrayIndexed64iv                );
    LOAD_GLPROC( glCreateSamplers                           );
    LOAD_GLPROC( glCreateProgramPipelines                   );
    LOAD_GLPROC( glCreateQueries                            );
    LOAD_GLPROC( glGetQueryBufferObjectiv                   );
    LOAD_GLPROC( glGetQueryBufferObjectuiv                  );
    LOAD_GLPROC( glGetQueryBufferObjecti64v                 );
    LOAD_GLPROC( glGetQueryBufferObjectui64v                );
    return true;
}

#undef DECL_LOADGLEXT_PROC
#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC

#endif // /ifndef(__APPLE__)


/* --- Common extension loading functions --- */

static GLExtensionMap QuerySupportedOpenGLExtensions(bool isCoreProfile)
{
    GLExtensionMap extensions;

    /* Filter standard GL extensions */
    if (isCoreProfile)
    {
        #if GL_VERSION_3_0 && !GL_GLEXT_PROTOTYPES

        #ifndef __APPLE__
        if (glGetStringi || LoadGLProc(glGetStringi, "glGetStringi"))
        #endif
        {
            /* Get number of extensions */
            GLint numExtensions = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

            for_range(i, numExtensions)
            {
                /* Get current extension string */
                if (const char* extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)))
                    extensions[extString] = false;
            }
        }

        #endif
    }
    else
    {
        /* Get complete extension string */
        if (const char* extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)))
            ExtractExtensionsFromString(extensions, extString);
    }

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
static void IncludeDefaultCoreProfileExtensions(GLExtensionMap& extensions)
{
    static const char* coreProfileDefaultExtenions[] =
    {
        "GL_ARB_compatibility",
        "GL_ARB_multitexture",          // GL 1.2
        "GL_ARB_shader_objects",
        "GL_ARB_shader_objects_21",
        "GL_ARB_shader_objects_30",
        "GL_ARB_vertex_buffer_object",
        "GL_ARB_vertex_shader",
        "GL_EXT_blend_func_separate",   // GL 2.0
        "GL_EXT_copy_texture",
        "GL_EXT_gpu_shader4",           // GL 2.0
        "GL_EXT_stencil_two_side",      // GL 2.0
        "GL_EXT_texture3D",
    };
    for (const char* ext : coreProfileDefaultExtenions)
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
    ImplyExtension("GL_ARB_gpu_shader5", { "GL_ARB_geometry_shader4" });
    ImplyExtension("GL_ARB_occlusion_query2", { "GL_ARB_occlusion_query" });
}

#endif // /__APPLE__

// Global member to store if the extension have already been loaded
static bool                     g_OpenGLExtensionsLoaded = false;
static GLExtensionMap           g_OpenGLExtensionsMap;
static std::set<const char*>    g_supportedOpenGLExtensions;
static std::set<const char*>    g_loadedOpenGLExtensions;

bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_OpenGLExtensionsLoaded)
        return true;

    /* Query supported OpenGL extension names */
    g_OpenGLExtensionsMap = QuerySupportedOpenGLExtensions(isCoreProfile);

    #ifdef __APPLE__

    /* Enable OpenGL extension support by host MacOS version */
    #define ENABLE_GLEXT(NAME) \
        RegisterExtension(GLExt::NAME)

    /* Enable basic GL functionality (such as glPrimitiveRestartIndex) */
    ENABLE_GLEXT( ARB_compatibility                );

    /* Enable hardware buffer extensions */
    ENABLE_GLEXT( ARB_vertex_buffer_object         );
    ENABLE_GLEXT( ARB_vertex_array_object          );
    ENABLE_GLEXT( ARB_vertex_shader                );
    ENABLE_GLEXT( ARB_framebuffer_object           );
    ENABLE_GLEXT( ARB_uniform_buffer_object        );
    ENABLE_GLEXT( ARB_map_buffer_range             );

    /* Enable drawing extensions */
    ENABLE_GLEXT( ARB_draw_instanced               );
    ENABLE_GLEXT( ARB_draw_elements_base_vertex    );

    /* Enable shader extensions */
    ENABLE_GLEXT( ARB_shader_objects               );
    ENABLE_GLEXT( ARB_instanced_arrays             );
    ENABLE_GLEXT( ARB_tessellation_shader          );
    ENABLE_GLEXT( ARB_get_program_binary           );
    ENABLE_GLEXT( ARB_program_interface_query      );
    ENABLE_GLEXT( ARB_separate_shader_objects      );
    ENABLE_GLEXT( EXT_gpu_shader4                  );

    /* Enable texture extensions */
    ENABLE_GLEXT( ARB_multitexture                 );
    ENABLE_GLEXT( EXT_texture3D                    );
    ENABLE_GLEXT( EXT_copy_texture                 );
    ENABLE_GLEXT( ARB_clear_texture                );
    ENABLE_GLEXT( ARB_texture_buffer_object        );
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
    ENABLE_GLEXT( EXT_texture_array                );
    ENABLE_GLEXT( ARB_texture_cube_map_array       );
    ENABLE_GLEXT( ARB_geometry_shader4             );

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
    if (isCoreProfile)
        IncludeDefaultCoreProfileExtensions(g_OpenGLExtensionsMap);

    IncludeImpliedExtensions(g_OpenGLExtensionsMap);

    #if defined(GL_VERSION_3_1) && !defined(GL_GLEXT_PROTOTYPES)
    LOAD_GLEXT( ARB_compatibility );
    #endif

    /* Load hardware buffer extensions */
    LOAD_GLEXT( ARB_vertex_buffer_object         ); // Always required for GL 3+
    LOAD_GLEXT( ARB_vertex_array_object          ); // Always required for GL 3+
    LOAD_GLEXT( ARB_vertex_shader                ); // Always required for GL 3+
    LOAD_GLEXT( ARB_framebuffer_object           ); // Always required for GL 2.x & GL 3+
    LOAD_GLEXT( ARB_uniform_buffer_object        );
    LOAD_GLEXT( ARB_shader_storage_buffer_object );
    LOAD_GLEXT( ARB_map_buffer_range             );

    /* Load drawing extensions */
    LOAD_GLEXT( ARB_draw_instanced               );
    LOAD_GLEXT( ARB_base_instance                );
    LOAD_GLEXT( ARB_draw_elements_base_vertex    );

    /* Load shader extensions */
    LOAD_GLEXT( ARB_shader_objects               );
    LOAD_GLEXT( ARB_shader_objects_21            ); //TODO: load if GL version is high enough
    LOAD_GLEXT( ARB_shader_objects_30            ); //TODO: load if GL version is high enough
    LOAD_GLEXT( ARB_shader_objects_40            ); //TODO: load if GL version is high enough
    LOAD_GLEXT( ARB_instanced_arrays             );
    LOAD_GLEXT( ARB_tessellation_shader          );
    LOAD_GLEXT( ARB_compute_shader               );
    LOAD_GLEXT( ARB_get_program_binary           );
    LOAD_GLEXT( ARB_program_interface_query      );
    LOAD_GLEXT( ARB_separate_shader_objects      );
    LOAD_GLEXT( EXT_gpu_shader4                  );

    /* Load texture extensions */
    LOAD_GLEXT( ARB_multitexture                 );
    LOAD_GLEXT( EXT_texture3D                    );
    LOAD_GLEXT( EXT_copy_texture                 );
    LOAD_GLEXT( ARB_clear_texture                );
    LOAD_GLEXT( ARB_texture_buffer_object        );
    LOAD_GLEXT( ARB_texture_buffer_range         );
    LOAD_GLEXT( ARB_texture_compression          );
    LOAD_GLEXT( ARB_texture_multisample          );
    LOAD_GLEXT( ARB_texture_view                 );
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
    LOAD_GLEXT( ARB_draw_buffers                 );
    LOAD_GLEXT( EXT_draw_buffers2                );
    LOAD_GLEXT( EXT_transform_feedback           );
    LOAD_GLEXT( NV_transform_feedback            );
    LOAD_GLEXT( ARB_transform_feedback2          );
    LOAD_GLEXT( ARB_sync                         );
    LOAD_GLEXT( ARB_internalformat_query         );
    LOAD_GLEXT( ARB_internalformat_query2        );
    LOAD_GLEXT( ARB_ES2_compatibility            );
    LOAD_GLEXT( ARB_gl_spirv                     );
    LOAD_GLEXT( ARB_texture_storage              );
    LOAD_GLEXT( ARB_texture_storage_multisample  );
    LOAD_GLEXT( ARB_buffer_storage               );
    LOAD_GLEXT( ARB_copy_buffer                  );
    LOAD_GLEXT( ARB_copy_image                   );
    LOAD_GLEXT( ARB_polygon_offset_clamp         );
    LOAD_GLEXT( ARB_shader_image_load_store      );
    LOAD_GLEXT( ARB_framebuffer_no_attachments   );
    LOAD_GLEXT( ARB_clear_buffer_object          );
    LOAD_GLEXT( ARB_draw_indirect                );
    LOAD_GLEXT( ARB_multi_draw_indirect          );
    LOAD_GLEXT( ARB_get_texture_sub_image        );
    #ifdef LLGL_GL_ENABLE_DSA_EXT
    LOAD_GLEXT( ARB_direct_state_access          );
    #endif

    /* Enable extensions and ignore procedures */
    ENABLE_GLEXT( ARB_transform_feedback3          ); // Only used for GL_MAX_TRANSFORM_FEEDBACK_BUFFERS

    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_geometry_shader4             );
    ENABLE_GLEXT( ARB_texture_cube_map             );
    ENABLE_GLEXT( ARB_texture_cube_map_array       );
    ENABLE_GLEXT( ARB_pipeline_statistics_query    );
    ENABLE_GLEXT( ARB_seamless_cubemap_per_texture );
    ENABLE_GLEXT( ARB_ES3_compatibility            );
    ENABLE_GLEXT( EXT_texture_array                );
    ENABLE_GLEXT( INTEL_conservative_rasterization );
    ENABLE_GLEXT( NV_conservative_raster           );

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
