/*
 * GLCoreExtensionLoader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../Ext/GLExtensionLoader.h"
#include "GLCoreExtensions.h"
#include "GLCoreExtensionsProxy.h"
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
    Log::PostReport(Log::ReportType::Error, "OS not supported for loading OpenGL extensions");
    return false;
    #endif

    /* Check for errors */
    if (!procAddr)
    {
        Log::PostReport(Log::ReportType::Error, "failed to load OpenGL procedure: " + std::string(procName));
        return false;
    }

    return true;
}

static void ExtractExtensionsFromString(GLExtensionList& extensions, const std::string& extString)
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

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS

#define LOAD_GLPROC(NAME)               \
    if (usePlaceholder)                 \
        NAME = Proxy_##NAME;            \
    else if (!LoadGLProc(NAME, #NAME))  \
        return false

#else

#define LOAD_GLPROC(NAME)           \
    if (!LoadGLProc(NAME, #NAME))   \
        return false

#endif // /LLGL_GL_ENABLE_EXT_PLACEHOLDERS

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

static bool Load_GL_ARB_compatibility(bool usePlaceholder)
{
    LOAD_GLPROC( glPrimitiveRestartIndex );
    return true;
}

#endif

/* --- Hardware buffer extensions --- */

static bool Load_GL_ARB_vertex_buffer_object(bool usePlaceholder)
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

static bool Load_GL_ARB_vertex_array_object(bool usePlaceholder)
{
    LOAD_GLPROC( glGenVertexArrays    );
    LOAD_GLPROC( glDeleteVertexArrays );
    LOAD_GLPROC( glBindVertexArray    );
    LOAD_GLPROC( glIsVertexArray      );
    return true;
}

static bool Load_GL_ARB_vertex_shader(bool usePlaceholder)
{
    LOAD_GLPROC( glEnableVertexAttribArray  );
    LOAD_GLPROC( glDisableVertexAttribArray );
    LOAD_GLPROC( glVertexAttribPointer      );
    LOAD_GLPROC( glBindAttribLocation       );
    return true;
}

static bool Load_GL_ARB_framebuffer_object(bool usePlaceholder)
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
    LOAD_GLPROC( glFramebufferTexture                  ); // <--- other extension! (but which one???)
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

static bool Load_GL_ARB_uniform_buffer_object(bool usePlaceholder)
{
    LOAD_GLPROC( glGetUniformBlockIndex      );
    LOAD_GLPROC( glGetActiveUniformBlockiv   );
    LOAD_GLPROC( glGetActiveUniformBlockName );
    LOAD_GLPROC( glUniformBlockBinding       );
    LOAD_GLPROC( glBindBufferBase            );
    return true;
}

static bool Load_GL_ARB_shader_storage_buffer_object(bool usePlaceholder)
{
    LOAD_GLPROC( glShaderStorageBlockBinding );
    return true;
}

/* --- Drawing extensions --- */

static bool Load_GL_ARB_draw_instanced(bool usePlaceholder)
{
    LOAD_GLPROC( glDrawArraysInstanced   );
    LOAD_GLPROC( glDrawElementsInstanced );
    return true;
}

static bool Load_GL_ARB_base_instance(bool usePlaceholder)
{
    LOAD_GLPROC( glDrawArraysInstancedBaseInstance             );
    LOAD_GLPROC( glDrawElementsInstancedBaseInstance           );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertexBaseInstance );
    return true;
}

static bool Load_GL_ARB_draw_elements_base_vertex(bool usePlaceholder)
{
    LOAD_GLPROC( glDrawElementsBaseVertex          );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertex );
    return true;
}

/* --- Shader extensions --- */

static bool Load_GL_ARB_shader_objects(bool usePlaceholder)
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

static bool Load_GL_ARB_shader_objects_21(bool usePlaceholder)
{
    LOAD_GLPROC( glUniformMatrix2x3fv );
    LOAD_GLPROC( glUniformMatrix2x4fv );
    LOAD_GLPROC( glUniformMatrix3x2fv );
    LOAD_GLPROC( glUniformMatrix3x4fv );
    LOAD_GLPROC( glUniformMatrix4x2fv );
    LOAD_GLPROC( glUniformMatrix4x3fv );
    return true;
}

static bool Load_GL_ARB_shader_objects_30(bool usePlaceholder)
{
    LOAD_GLPROC( glUniform1uiv );
    LOAD_GLPROC( glUniform2uiv );
    LOAD_GLPROC( glUniform3uiv );
    LOAD_GLPROC( glUniform4uiv );
    return true;
}

static bool Load_GL_ARB_shader_objects_40(bool usePlaceholder)
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

static bool Load_GL_ARB_instanced_arrays(bool usePlaceholder)
{
    LOAD_GLPROC( glVertexAttribDivisor );
    return true;
}

static bool Load_GL_ARB_tessellation_shader(bool usePlaceholder)
{
    LOAD_GLPROC( glPatchParameteri  );
    LOAD_GLPROC( glPatchParameterfv );
    return true;
}

static bool Load_GL_ARB_compute_shader(bool usePlaceholder)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}

static bool Load_GL_ARB_get_program_binary(bool usePlaceholder)
{
    LOAD_GLPROC( glGetProgramBinary  );
    LOAD_GLPROC( glProgramBinary     );
    LOAD_GLPROC( glProgramParameteri );
    return true;
}

static bool Load_GL_ARB_program_interface_query(bool usePlaceholder)
{
    LOAD_GLPROC( glGetProgramInterfaceiv           );
    LOAD_GLPROC( glGetProgramResourceIndex         );
    LOAD_GLPROC( glGetProgramResourceName          );
    LOAD_GLPROC( glGetProgramResourceiv            );
    LOAD_GLPROC( glGetProgramResourceLocation      );
    LOAD_GLPROC( glGetProgramResourceLocationIndex );
    return true;
}

static bool Load_GL_EXT_gpu_shader4(bool usePlaceholder)
{
    LOAD_GLPROC( glVertexAttribIPointer );
    LOAD_GLPROC( glBindFragDataLocation );
    LOAD_GLPROC( glGetFragDataLocation  );
    return true;
}

/* --- Texture extensions --- */

static bool Load_GL_ARB_multitexture(bool usePlaceholder)
{
    LOAD_GLPROC( glActiveTexture );
    return true;
}

static bool Load_GL_EXT_texture3D(bool usePlaceholder)
{
    LOAD_GLPROC( glTexImage3D    );
    LOAD_GLPROC( glTexSubImage3D );
    return true;
}

static bool Load_GL_EXT_copy_texture(bool usePlaceholder)
{
    LOAD_GLPROC( glCopyTexSubImage3D );
    return true;
}

static bool Load_GL_ARB_clear_texture(bool usePlaceholder)
{
    LOAD_GLPROC( glClearTexImage    );
    LOAD_GLPROC( glClearTexSubImage );
    return true;
}

static bool Load_GL_ARB_texture_compression(bool usePlaceholder)
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

static bool Load_GL_ARB_texture_multisample(bool usePlaceholder)
{
    LOAD_GLPROC( glTexImage2DMultisample );
    LOAD_GLPROC( glTexImage3DMultisample );
    LOAD_GLPROC( glGetMultisamplefv      );
    LOAD_GLPROC( glSampleMaski           );
    return true;
}

static bool Load_GL_ARB_texture_view(bool usePlaceholder)
{
    LOAD_GLPROC( glTextureView );
    return true;
}

static bool Load_GL_ARB_sampler_objects(bool usePlaceholder)
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

static bool Load_GL_ARB_occlusion_query(bool usePlaceholder)
{
    LOAD_GLPROC( glGenQueries        );
    LOAD_GLPROC( glDeleteQueries     );
    LOAD_GLPROC( glBeginQuery        );
    LOAD_GLPROC( glEndQuery          );
    LOAD_GLPROC( glGetQueryObjectiv  );
    LOAD_GLPROC( glGetQueryObjectuiv );
    return true;
}

static bool Load_GL_NV_conditional_render(bool usePlaceholder)
{
    LOAD_GLPROC( glBeginConditionalRender );
    LOAD_GLPROC( glEndConditionalRender   );
    return true;
}

static bool Load_GL_ARB_timer_query(bool usePlaceholder)
{
    LOAD_GLPROC( glQueryCounter        );
    LOAD_GLPROC( glGetQueryObjecti64v  );
    LOAD_GLPROC( glGetQueryObjectui64v );
    return true;
}

static bool Load_GL_ARB_viewport_array(bool usePlaceholder)
{
    LOAD_GLPROC( glViewportArrayv   );
    LOAD_GLPROC( glScissorArrayv    );
    LOAD_GLPROC( glDepthRangeArrayv );
    return true;
}

static bool Load_GL_EXT_blend_minmax(bool usePlaceholder)
{
    LOAD_GLPROC( glBlendEquation );
    return true;
}

static bool Load_GL_EXT_blend_color(bool usePlaceholder)
{
    LOAD_GLPROC( glBlendColor );
    return true;
}

static bool Load_GL_EXT_blend_func_separate(bool usePlaceholder)
{
    LOAD_GLPROC( glBlendFuncSeparate );
    return true;
}

static bool Load_GL_EXT_blend_equation_separate(bool usePlaceholder)
{
    LOAD_GLPROC( glBlendEquationSeparate );
    return true;
}

static bool Load_GL_ARB_draw_buffers_blend(bool usePlaceholder)
{
    LOAD_GLPROC( glBlendEquationi         );
    LOAD_GLPROC( glBlendEquationSeparatei );
    LOAD_GLPROC( glBlendFunci             );
    LOAD_GLPROC( glBlendFuncSeparatei     );
    return true;
}

static bool Load_GL_ARB_multi_bind(bool usePlaceholder)
{
    LOAD_GLPROC( glBindBuffersBase   );
    LOAD_GLPROC( glBindBuffersRange  );
    LOAD_GLPROC( glBindTextures      );
    LOAD_GLPROC( glBindSamplers      );
    LOAD_GLPROC( glBindImageTextures );
    LOAD_GLPROC( glBindVertexBuffers );
    return true;
}

static bool Load_GL_EXT_stencil_two_side(bool usePlaceholder)
{
    //correct extension ??? maybe "GL_ATI_separate_stencil"
    LOAD_GLPROC( glStencilFuncSeparate );
    LOAD_GLPROC( glStencilMaskSeparate );
    LOAD_GLPROC( glStencilOpSeparate   );
    return true;
}

static bool Load_GL_KHR_debug(bool usePlaceholder)
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

static bool Load_GL_ARB_clip_control(bool usePlaceholder)
{
    LOAD_GLPROC( glClipControl );
    return true;
}

static bool Load_GL_ARB_draw_buffers(bool usePlaceholder)
{
    LOAD_GLPROC( glDrawBuffers );
    return true;
}

static bool Load_GL_EXT_draw_buffers2(bool usePlaceholder)
{
    LOAD_GLPROC( glColorMaski    );
    LOAD_GLPROC( glGetBooleani_v );
    LOAD_GLPROC( glGetIntegeri_v );
    LOAD_GLPROC( glEnablei       );
    LOAD_GLPROC( glDisablei      );
    LOAD_GLPROC( glIsEnabledi    );
    return true;
}

static bool Load_GL_EXT_transform_feedback(bool usePlaceholder)
{
    LOAD_GLPROC( glBindBufferRange             );
    LOAD_GLPROC( glBeginTransformFeedback      );
    LOAD_GLPROC( glEndTransformFeedback        );
    LOAD_GLPROC( glTransformFeedbackVaryings   );
    LOAD_GLPROC( glGetTransformFeedbackVarying );
    return true;
}

static bool Load_GL_NV_transform_feedback(bool usePlaceholder)
{
    LOAD_GLPROC( glBindBufferRangeNV           );
    LOAD_GLPROC( glBeginTransformFeedbackNV    );
    LOAD_GLPROC( glEndTransformFeedbackNV      );
    LOAD_GLPROC( glTransformFeedbackVaryingsNV );
    LOAD_GLPROC( glGetVaryingLocationNV        );
    LOAD_GLPROC( glGetActiveVaryingNV          );
    return true;
}

static bool Load_GL_ARB_sync(bool usePlaceholder)
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

static bool Load_GL_ARB_internalformat_query(bool usePlaceholder)
{
    LOAD_GLPROC( glGetInternalformativ );
    return true;
}

static bool Load_GL_ARB_internalformat_query2(bool usePlaceholder)
{
    LOAD_GLPROC( glGetInternalformati64v );
    return true;
}

static bool Load_GL_ARB_ES2_compatibility(bool usePlaceholder)
{
    LOAD_GLPROC( glReleaseShaderCompiler    );
    LOAD_GLPROC( glShaderBinary             );
    LOAD_GLPROC( glGetShaderPrecisionFormat );
    LOAD_GLPROC( glDepthRangef              );
    LOAD_GLPROC( glClearDepthf              );
    return true;
}

static bool Load_GL_ARB_gl_spirv(bool usePlaceholder)
{
    LOAD_GLPROC( glSpecializeShader );
    return true;
}

static bool Load_GL_ARB_texture_storage(bool usePlaceholder)
{
    LOAD_GLPROC( glTexStorage1D );
    LOAD_GLPROC( glTexStorage2D );
    LOAD_GLPROC( glTexStorage3D );
    return true;
}

static bool Load_GL_ARB_texture_storage_multisample(bool usePlaceholder)
{
    LOAD_GLPROC( glTexStorage2DMultisample );
    LOAD_GLPROC( glTexStorage3DMultisample );
    return true;
}

static bool Load_GL_ARB_buffer_storage(bool usePlaceholder)
{
    LOAD_GLPROC( glBufferStorage );
    return true;
}

static bool Load_GL_ARB_copy_buffer(bool usePlaceholder)
{
    LOAD_GLPROC( glCopyBufferSubData );
    return true;
}

static bool Load_GL_ARB_copy_image(bool usePlaceholder)
{
    LOAD_GLPROC( glCopyImageSubData );
    return true;
}

static bool Load_GL_ARB_polygon_offset_clamp(bool usePlaceholder)
{
    LOAD_GLPROC( glPolygonOffsetClamp );
    return true;
}

static bool Load_GL_ARB_shader_image_load_store(bool usePlaceholder)
{
    LOAD_GLPROC( glBindImageTexture );
    LOAD_GLPROC( glMemoryBarrier    );
    return true;
}

static bool Load_GL_ARB_framebuffer_no_attachments(bool usePlaceholder)
{
    LOAD_GLPROC( glFramebufferParameteri     );
    LOAD_GLPROC( glGetFramebufferParameteriv );
    return true;
}

static bool Load_GL_ARB_clear_buffer_object(bool usePlaceholder)
{
    LOAD_GLPROC( glClearBufferData    );
    LOAD_GLPROC( glClearBufferSubData );
    return true;
}

static bool Load_GL_ARB_draw_indirect(bool usePlaceholder)
{
    LOAD_GLPROC( glDrawArraysIndirect   );
    LOAD_GLPROC( glDrawElementsIndirect );
    return true;
}

static bool Load_GL_ARB_multi_draw_indirect(bool usePlaceholder)
{
    LOAD_GLPROC( glMultiDrawArraysIndirect   );
    LOAD_GLPROC( glMultiDrawElementsIndirect );
    return true;
}

static bool Load_GL_ARB_get_texture_sub_image(bool usePlaceholder)
{
    LOAD_GLPROC( glGetTextureSubImage           );
    LOAD_GLPROC( glGetCompressedTextureSubImage );
    return true;
}

static bool Load_GL_ARB_direct_state_access(bool usePlaceholder)
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

#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC

#endif // /ifndef(__APPLE__)


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
                    extensions[extString] = false;
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

void LoadAllExtensions(GLExtensionList& extensions, bool coreProfile)
{
    /* Only load GL extensions once */
    if (g_extAlreadyLoaded)
        return;

    #ifdef __APPLE__

    /* Enable OpenGL extension support by host MacOS version */
    #define ENABLE_GLEXT(NAME) \
        RegisterExtension(GLExt::NAME)

    /* Enable hardware buffer extensions */
    ENABLE_GLEXT( ARB_vertex_buffer_object         );
    ENABLE_GLEXT( ARB_vertex_array_object          );
    ENABLE_GLEXT( ARB_vertex_shader                );
    ENABLE_GLEXT( ARB_framebuffer_object           );
    ENABLE_GLEXT( ARB_uniform_buffer_object        );

    /* Enable drawing extensions */
    ENABLE_GLEXT( ARB_draw_instanced               );
    ENABLE_GLEXT( ARB_draw_elements_base_vertex    );

    /* Enable shader extensions */
    ENABLE_GLEXT( ARB_shader_objects               );
    ENABLE_GLEXT( ARB_instanced_arrays             );
    ENABLE_GLEXT( ARB_tessellation_shader          );
    ENABLE_GLEXT( ARB_get_program_binary           );
    ENABLE_GLEXT( ARB_program_interface_query      );
    ENABLE_GLEXT( EXT_gpu_shader4                  );

    /* Enable texture extensions */
    ENABLE_GLEXT( ARB_multitexture                 );
    ENABLE_GLEXT( EXT_texture3D                    );
    ENABLE_GLEXT( EXT_copy_texture                 );
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

    #else

    auto LoadExtension = [&](const std::string& extName, const std::function<bool(bool)>& extLoadingProc, GLExt extensionID) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extensions.find(extName);
        if (it != extensions.end())
        {
            if (extLoadingProc(false))
            {
                /* Enable extension in registry */
                RegisterExtension(extensionID);
                it->second = true;
            }
            else
            {
                /* Loading extension failed */
                Log::PostReport(Log::ReportType::Error, "failed to load OpenGL extension: " + extName);
            }
        }
        #ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS
        else
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(true);
        }
        #endif
    };

    auto EnableExtension = [&](const std::string& extName, GLExt extensionID) -> void
    {
        /* Try to enable OpenGL extension */
        if (extensions.find(extName) != extensions.end())
            RegisterExtension(extensionID);
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" + std::string(#NAME), Load_GL_##NAME, GLExt::NAME)

    #define ENABLE_GLEXT(NAME) \
        EnableExtension("GL_" + std::string(#NAME), GLExt::NAME)

    /* Add standard extensions */
    if (coreProfile)
    {
        static const std::string coreProfileDefaultExtenions[] =
        {
            "GL_ARB_compatibility",
            "GL_ARB_shader_objects",
            "GL_ARB_shader_objects_21",
            "GL_ARB_shader_objects_30",
            "GL_ARB_vertex_buffer_object",
            "GL_ARB_vertex_shader",
            "GL_EXT_texture3D",
            "GL_EXT_copy_texture",
        };
        for (const auto& ext : coreProfileDefaultExtenions)
            extensions[ext] = false;
    }

    #if defined(GL_VERSION_3_1) && !defined(GL_GLEXT_PROTOTYPES)
    LOAD_GLEXT( ARB_compatibility );
    #endif

    /* Load hardware buffer extensions */
    LOAD_GLEXT( ARB_vertex_buffer_object         );
    LOAD_GLEXT( ARB_vertex_array_object          );
    LOAD_GLEXT( ARB_vertex_shader                );
    LOAD_GLEXT( ARB_framebuffer_object           );
    LOAD_GLEXT( ARB_uniform_buffer_object        );
    LOAD_GLEXT( ARB_shader_storage_buffer_object );

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
    LOAD_GLEXT( EXT_gpu_shader4                  );

    /* Load texture extensions */
    LOAD_GLEXT( ARB_multitexture                 );
    LOAD_GLEXT( EXT_texture3D                    );
    LOAD_GLEXT( EXT_copy_texture                 );
    LOAD_GLEXT( ARB_clear_texture                );
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
    ENABLE_GLEXT( ARB_transform_feedback3 );

    /* Enable extensions without procedures */
    ENABLE_GLEXT( ARB_texture_cube_map             );
    ENABLE_GLEXT( EXT_texture_array                );
    ENABLE_GLEXT( ARB_texture_cube_map_array       );
    ENABLE_GLEXT( ARB_geometry_shader4             );
    ENABLE_GLEXT( NV_conservative_raster           );
    ENABLE_GLEXT( INTEL_conservative_rasterization );
    ENABLE_GLEXT( ARB_pipeline_statistics_query    );

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
