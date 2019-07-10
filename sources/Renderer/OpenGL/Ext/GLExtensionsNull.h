/*
 * GLExtensionsNull.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS


// THIS FILE MUST NOT HAVE A HEADER GUARD

/*
All OpenGL extension placeholder procedures are defined here.
In case that an GL extension is not supported, the respective
function pointers are set to their respective dummy functions.
These functions always throw an 'std::runtime_error' exception,
to notify the client programmer about an illegal use of an
unsupported GL extension.
*/


#include "../OpenGL.h"


namespace LLGL
{


#ifdef LLGL_DEF_GL_DUMMY_PROCS

#define DECL_GLPROC(RTYPE, NAME, ARGS)  \
    RTYPE APIENTRY Dummy_##NAME ARGS    \
    {                                   \
        ErrUnsupportedGLProc(#NAME);    \
    }

#else

#define DECL_GLPROC(RTYPE, NAME, ARGS) \
    RTYPE APIENTRY Dummy_##NAME ARGS

#endif

#ifndef __APPLE__

/* Platform specific GL extensions */

#if defined(_WIN32)

// WGL_EXT_swap_control
DECL_GLPROC(BOOL, wglSwapIntervalEXT, (int));
DECL_GLPROC(BOOL, wglChoosePixelFormatARB, (HDC, const int*, const FLOAT*, UINT, int*, UINT*));
DECL_GLPROC(HGLRC, wglCreateContextAttribsARB, (HDC, HGLRC, const int*));
DECL_GLPROC(const char*, wglGetExtensionsStringARB, (HDC));

#elif defined(__linux__)

// GLX_SGI_swap_control
DECL_GLPROC(int, glXSwapIntervalSGI, (int));

#endif

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

/* GL 3.0 extensions (for Core Profile) */

DECL_GLPROC(const GLubyte*, glGetStringi, (GLenum GLuint));

#endif

/* GL_EXT_blend_func_separate */

DECL_GLPROC(void, glBlendFuncSeparate, (GLenum, GLenum, GLenum, GLenum));

/* GL_EXT_blend_minmax */

DECL_GLPROC(void, glBlendEquation, (GLenum));

/* GL_EXT_blend_color */

DECL_GLPROC(void, glBlendColor, (GLfloat, GLfloat, GLfloat, GLfloat));

/* GL_EXT_blend_equation_separate */

DECL_GLPROC(void, glBlendEquationSeparate, (GLenum, GLenum));

/* GL_ARB_draw_buffers */

DECL_GLPROC(void, glDrawBuffers, (GLsizei, const GLenum*));

/* GL_EXT_draw_buffers2 */

DECL_GLPROC(void, glColorMaski, (GLuint, GLboolean, GLboolean, GLboolean, GLboolean));
DECL_GLPROC(void, glGetBooleani_v, (GLenum, GLuint, GLboolean*));
DECL_GLPROC(void, glGetIntegeri_v, (GLenum, GLuint, GLint*));
DECL_GLPROC(void, glEnablei, (GLenum, GLuint));
DECL_GLPROC(void, glDisablei, (GLenum, GLuint));
DECL_GLPROC(GLboolean, glIsEnabledi, (GLenum, GLuint));

/* GL_ARB_draw_buffers_blend */

DECL_GLPROC(void, glBlendEquationi, (GLuint, GLenum));
DECL_GLPROC(void, glBlendEquationSeparatei, (GLuint, GLenum, GLenum));
DECL_GLPROC(void, glBlendFunci, (GLuint, GLenum, GLenum));
DECL_GLPROC(void, glBlendFuncSeparatei, (GLuint, GLenum, GLenum, GLenum, GLenum));

/* GL_ARB_multitexture */

DECL_GLPROC(void, glActiveTexture, (GLenum));

/* GL_EXT_texture3D */

DECL_GLPROC(void, glTexImage3D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(void, glTexSubImage3D, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* GL_ARB_clear_texture */

DECL_GLPROC(void, glClearTexImage, (GLuint, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(void, glClearTexSubImage, (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* GL_ARB_texture_compression */

DECL_GLPROC(void, glCompressedTexImage1D, (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexImage2D, (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexImage3D, (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage1D, (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage3D, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glGetCompressedTexImage, (GLenum, GLint, void*));

/* GL_ARB_texture_multisample */

DECL_GLPROC(void, glTexImage2DMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(void, glTexImage3DMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(void, glGetMultisamplefv, (GLenum, GLuint, GLfloat*));
DECL_GLPROC(void, glSampleMaski, (GLuint, GLbitfield));

/* GL_ARB_sampler_objects */

DECL_GLPROC(void, glGenSamplers, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteSamplers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindSampler, (GLuint, GLuint));
DECL_GLPROC(void, glSamplerParameteri, (GLuint, GLenum, GLint));
DECL_GLPROC(void, glSamplerParameterf, (GLuint, GLenum, GLfloat));
DECL_GLPROC(void, glSamplerParameteriv, (GLuint, GLenum, const GLint*));
DECL_GLPROC(void, glSamplerParameterfv, (GLuint, GLenum, const GLfloat*));

/* GL_ARB_multi_bind */

DECL_GLPROC(void, glBindBuffersBase, (GLenum, GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindBuffersRange, (GLenum, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizeiptr*));
DECL_GLPROC(void, glBindTextures, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindSamplers, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindImageTextures, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindVertexBuffers, (GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*));

/* GL_ARB_vertex_buffer_object */

DECL_GLPROC(void, glGenBuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteBuffers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindBuffer, (GLenum, GLuint));
DECL_GLPROC(GLboolean, glIsBuffer, (GLuint));
DECL_GLPROC(void, glBufferData, (GLenum, GLsizeiptr, const void*, GLenum));
DECL_GLPROC(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const void*));
DECL_GLPROC(void, glGetBufferSubData, (GLenum, GLintptr, GLsizeiptr, void*));
DECL_GLPROC(void*, glMapBuffer, (GLenum, GLenum));
DECL_GLPROC(GLboolean, glUnmapBuffer, (GLenum));
DECL_GLPROC(void, glGetBufferParameteriv, (GLenum, GLenum, GLint*));
DECL_GLPROC(void, glGetBufferPointerv, (GLenum, GLenum, void**));

/* GL_ARB_vertex_shader */

DECL_GLPROC(void, glEnableVertexAttribArray, (GLuint));
DECL_GLPROC(void, glDisableVertexAttribArray, (GLuint));
DECL_GLPROC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*));
DECL_GLPROC(void, glBindAttribLocation, (GLuint, GLuint, const GLchar*));

/* GL_EXT_gpu_shader4 */

DECL_GLPROC(void, glVertexAttribIPointer, (GLuint, GLint, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glBindFragDataLocation, (GLuint, GLuint, const GLchar*));
DECL_GLPROC(GLint, glGetFragDataLocation, (GLuint, const GLchar*));

/* GL_ARB_instanced_arrays */

DECL_GLPROC(void, glVertexAttribDivisor, (GLuint, GLuint));

/* GL_ARB_vertex_array_object */

DECL_GLPROC(void, glGenVertexArrays, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteVertexArrays, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindVertexArray, (GLuint));
DECL_GLPROC(GLboolean, glIsVertexArray, (GLuint));

/* GL_ARB_framebuffer_object */

DECL_GLPROC(void, glGenRenderbuffers, (GLsizei n, GLuint *));
DECL_GLPROC(void, glDeleteRenderbuffers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindRenderbuffer, (GLenum, GLuint));
DECL_GLPROC(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glRenderbufferStorageMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glGenFramebuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteFramebuffers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindFramebuffer, (GLenum, GLuint));
DECL_GLPROC(GLenum, glCheckFramebufferStatus, (GLenum));
DECL_GLPROC(void, glFramebufferTexture, (GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(void, glFramebufferTexture1D, (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(void, glFramebufferTexture3D, (GLenum, GLenum, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(void, glFramebufferTextureLayer, (GLenum, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint));
DECL_GLPROC(void, glGetFramebufferAttachmentParameteriv, (GLenum, GLenum, GLenum, GLint*));
DECL_GLPROC(void, glBlitFramebuffer, (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum));
DECL_GLPROC(void, glGenerateMipmap, (GLenum));

#if 1 //WHICH EXTENSION???
DECL_GLPROC(void, glClearBufferiv, (GLenum, GLint, const GLint*));
DECL_GLPROC(void, glClearBufferuiv, (GLenum, GLint, const GLuint*));
DECL_GLPROC(void, glClearBufferfv, (GLenum, GLint, const GLfloat*));
DECL_GLPROC(void, glClearBufferfi, (GLenum, GLint, GLfloat, GLint));
#endif

/* GL_ARB_draw_instanced */

DECL_GLPROC(void, glDrawArraysInstanced, (GLenum, GLint, GLsizei, GLsizei));
DECL_GLPROC(void, glDrawElementsInstanced, (GLenum, GLsizei, GLenum, const void*, GLsizei));

/* GL_ARB_draw_elements_base_vertex */

DECL_GLPROC(void, glDrawElementsBaseVertex, (GLenum, GLsizei, GLenum, const void*, GLint));
DECL_GLPROC(void, glDrawElementsInstancedBaseVertex, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint));

/* GL_ARB_base_instance */

DECL_GLPROC(void, glDrawArraysInstancedBaseInstance, (GLenum, GLint, GLsizei, GLsizei, GLuint));
DECL_GLPROC(void, glDrawElementsInstancedBaseInstance, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLuint));
DECL_GLPROC(void, glDrawElementsInstancedBaseVertexBaseInstance, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint, GLuint));

/* GL_ARB_shader_objects */

DECL_GLPROC(GLuint, glCreateShader, (GLenum));
DECL_GLPROC(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint*));
DECL_GLPROC(void, glCompileShader, (GLuint));
DECL_GLPROC(void, glGetShaderiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glDeleteShader, (GLuint));
DECL_GLPROC(GLuint, glCreateProgram, (void));
DECL_GLPROC(void, glDeleteProgram, (GLuint));
DECL_GLPROC(void, glAttachShader, (GLuint, GLuint));
DECL_GLPROC(void, glDetachShader, (GLuint, GLuint));
DECL_GLPROC(void, glLinkProgram, (GLuint));
DECL_GLPROC(void, glValidateProgram, (GLuint));
DECL_GLPROC(void, glGetProgramiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glUseProgram, (GLuint));
DECL_GLPROC(void, glGetActiveAttrib, (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*));
DECL_GLPROC(GLint, glGetAttribLocation, (GLuint, const GLchar*));
DECL_GLPROC(void, glGetActiveUniform, (GLhandleARB, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLcharARB*));
DECL_GLPROC(GLint, glGetUniformLocation, (GLuint, const GLchar*));
DECL_GLPROC(void, glGetAttachedShaders, (GLuint, GLsizei, GLsizei*, GLuint*));
DECL_GLPROC(void, glUniform1f, (GLint, GLfloat));
DECL_GLPROC(void, glUniform2f, (GLint, GLfloat, GLfloat));
DECL_GLPROC(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat));
DECL_GLPROC(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat));
DECL_GLPROC(void, glUniform1i, (GLint, GLint));
DECL_GLPROC(void, glUniform2i, (GLint, GLint, GLint));
DECL_GLPROC(void, glUniform3i, (GLint, GLint, GLint, GLint));
DECL_GLPROC(void, glUniform4i, (GLint, GLint, GLint, GLint, GLint));
DECL_GLPROC(void, glUniform1fv, (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glUniform2fv, (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glUniform3fv, (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glUniform4fv, (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glUniform1iv, (GLint, GLsizei, const GLint*));
DECL_GLPROC(void, glUniform2iv, (GLint, GLsizei, const GLint*));
DECL_GLPROC(void, glUniform3iv, (GLint, GLsizei, const GLint*));
DECL_GLPROC(void, glUniform4iv, (GLint, GLsizei, const GLint*));
DECL_GLPROC(void, glUniformMatrix2fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix3fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glGetUniformiv, (GLuint, GLint, GLint*));
DECL_GLPROC(void, glGetUniformfv, (GLuint, GLint, GLfloat*));

/* **GL_ARB_shader_objects** TODO: which extension from GL 2.1 ??? */

DECL_GLPROC(void, glUniformMatrix2x3fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix2x4fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix3x2fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix3x4fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix4x2fv, (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(void, glUniformMatrix4x3fv, (GLint, GLsizei, GLboolean, const GLfloat*));

/* **GL_ARB_shader_objects** TODO: which extension from GL 3.0 ??? */

DECL_GLPROC(void, glUniform1uiv, (GLint, GLsizei, const GLuint*));
DECL_GLPROC(void, glUniform2uiv, (GLint, GLsizei, const GLuint*));
DECL_GLPROC(void, glUniform3uiv, (GLint, GLsizei, const GLuint*));
DECL_GLPROC(void, glUniform4uiv, (GLint, GLsizei, const GLuint*));

/* **GL_ARB_shader_objects** TODO: which extension from GL 4.0 ??? */

DECL_GLPROC(void, glUniform1dv, (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(void, glUniform2dv, (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(void, glUniform3dv, (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(void, glUniform4dv, (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix2dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix3dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix4dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix2x3dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix2x4dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix3x2dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix3x4dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix4x2dv, (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(void, glUniformMatrix4x3dv, (GLint, GLsizei, GLboolean, const GLdouble*));

/* GL_ARB_tessellation_shader */

DECL_GLPROC(void, glPatchParameteri, (GLenum, GLint));
DECL_GLPROC(void, glPatchParameterfv, (GLenum, const GLfloat*));

/* GL_ARB_compute_shader */

DECL_GLPROC(void, glDispatchCompute, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glDispatchComputeIndirect, (GLintptr));

/* GL_ARB_get_program_binary */

DECL_GLPROC(void, glGetProgramBinary, (GLuint, GLsizei, GLsizei*, GLenum*, void*));
DECL_GLPROC(void, glProgramBinary, (GLuint, GLenum, const void*, GLsizei));
DECL_GLPROC(void, glProgramParameteri, (GLuint, GLenum, GLint));

/* GL_ARB_program_interface_query */

DECL_GLPROC(void, glGetProgramInterfaceiv, (GLuint, GLenum, GLenum, GLint*));
DECL_GLPROC(GLuint, glGetProgramResourceIndex, (GLuint, GLenum, const GLchar*));
DECL_GLPROC(void, glGetProgramResourceName, (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glGetProgramResourceiv, (GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLint*));
DECL_GLPROC(GLint, glGetProgramResourceLocation, (GLuint, GLenum, const GLchar*));
DECL_GLPROC(GLint, glGetProgramResourceLocationIndex, (GLuint, GLenum, const GLchar*));

/* GL_ARB_uniform_buffer_object */

DECL_GLPROC(GLuint, glGetUniformBlockIndex, (GLuint, const GLchar*));
DECL_GLPROC(void, glGetActiveUniformBlockiv, (GLuint, GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetActiveUniformBlockName, (GLuint, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glUniformBlockBinding, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glBindBufferBase, (GLenum, GLuint, GLuint));

/* GL_ARB_shader_storage_buffer_object */

DECL_GLPROC(void, glShaderStorageBlockBinding, (GLuint, GLuint, GLuint));

/* GL_ARB_occlusion_query */

DECL_GLPROC(void, glGenQueries, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteQueries, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBeginQuery, (GLenum, GLuint));
DECL_GLPROC(void, glEndQuery, (GLenum));
DECL_GLPROC(void, glGetQueryObjectiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetQueryObjectuiv, (GLuint, GLenum, GLuint*));

/* GL_NV_conditional_render */

DECL_GLPROC(void, glBeginConditionalRender, (GLuint, GLenum));
DECL_GLPROC(void, glEndConditionalRender, (void));

/* GL_ARB_timer_query */

DECL_GLPROC(void, glQueryCounter, (GLuint, GLenum));
DECL_GLPROC(void, glGetQueryObjecti64v, (GLuint, GLenum, GLint64*));
DECL_GLPROC(void, glGetQueryObjectui64v, (GLuint, GLenum, GLuint64*));

/* GL_ARB_viewport_array */

DECL_GLPROC(void, glViewportArrayv, (GLuint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glScissorArrayv, (GLuint, GLsizei, const GLint*));
DECL_GLPROC(void, glDepthRangeArrayv, (GLuint, GLsizei, const GLdouble*));

/* GL_ATI_separate_stencil ??? */

DECL_GLPROC(void, glStencilFuncSeparate, (GLenum, GLenum, GLint, GLuint));
DECL_GLPROC(void, glStencilMaskSeparate, (GLenum, GLuint));
DECL_GLPROC(void, glStencilOpSeparate, (GLenum, GLenum, GLenum, GLenum));

/* GL_KHR_debug */

DECL_GLPROC(void, glDebugMessageCallback, (GLDEBUGPROC, const void*));

/* GL_ARB_clip_control */

DECL_GLPROC(void, glClipControl, (GLenum, GLenum));

/* GL_EXT_transform_feedback */

DECL_GLPROC(void, glBindBufferRange, (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glBeginTransformFeedback, (GLenum));
DECL_GLPROC(void, glEndTransformFeedback, (void));
DECL_GLPROC(void, glTransformFeedbackVaryings, (GLuint, GLsizei, const GLchar *const*, GLenum));
DECL_GLPROC(void, glGetTransformFeedbackVarying, (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*));

/* GL_NV_transform_feedback */

DECL_GLPROC(void, glBindBufferRangeNV, (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glBeginTransformFeedbackNV, (GLenum));
DECL_GLPROC(void, glEndTransformFeedbackNV, (void));
DECL_GLPROC(void, glTransformFeedbackVaryingsNV, (GLuint, GLsizei, const GLint*, GLenum));
DECL_GLPROC(GLint, glGetVaryingLocationNV, (GLuint, const GLchar*));
DECL_GLPROC(void, glGetActiveVaryingNV, (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*));

/* GL_ARB_sync */

DECL_GLPROC(GLsync, glFenceSync, (GLenum, GLbitfield));
DECL_GLPROC(GLboolean, glIsSync, (GLsync));
DECL_GLPROC(void, glDeleteSync, (GLsync));
DECL_GLPROC(GLenum, glClientWaitSync, (GLsync, GLbitfield, GLuint64));
DECL_GLPROC(void, glWaitSync, (GLsync, GLbitfield, GLuint64));
DECL_GLPROC(void, glGetInteger64v, (GLenum pname, GLint64*));
DECL_GLPROC(void, glGetSynciv, (GLsync, GLenum, GLsizei, GLsizei*, GLint*));

/* GL_ARB_internalformat_query */

DECL_GLPROC(void, glGetInternalformativ, (GLenum, GLenum, GLenum, GLsizei, GLint*));

/* GL_ARB_internalformat_query2 */

DECL_GLPROC(void, glGetInternalformati64v, (GLenum, GLenum, GLenum, GLsizei, GLint64*));

/* GL_ARB_ES2_compatibility */

DECL_GLPROC(void, glReleaseShaderCompiler, (void));
DECL_GLPROC(void, glShaderBinary, (GLsizei, const GLuint*, GLenum, const void*, GLsizei));
DECL_GLPROC(void, glGetShaderPrecisionFormat, (GLenum, GLenum, GLint*, GLint*));
DECL_GLPROC(void, glDepthRangef, (GLclampf, GLclampf));
DECL_GLPROC(void, glClearDepthf, (GLclampf));

/* GL_ARB_gl_spirv */

DECL_GLPROC(void, glSpecializeShader, (GLuint, const GLchar*, GLuint, const GLuint*, const GLuint*));

/* GL_ARB_texture_storage */

DECL_GLPROC(void, glTexStorage1D, (GLenum, GLsizei, GLenum, GLsizei));
DECL_GLPROC(void, glTexStorage2D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glTexStorage3D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei));

/* GL_ARB_texture_storage_multisample */

DECL_GLPROC(void, glTexStorage2DMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(void, glTexStorage3DMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));

/* GL_ARB_buffer_storage */

DECL_GLPROC(void, glBufferStorage, (GLenum, GLsizeiptr, const void*, GLbitfield));

/* ARB_copy_buffer */

DECL_GLPROC(void, glCopyBufferSubData, (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr));

/* GL_ARB_polygon_offset_clamp */

DECL_GLPROC(void, glPolygonOffsetClamp, (GLfloat, GLfloat, GLfloat));

/* GL_ARB_texture_view */

DECL_GLPROC(void, glTextureView, (GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint));

/* GL_ARB_shader_image_load_store */

DECL_GLPROC(void, glBindImageTexture, (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum));
DECL_GLPROC(void, glMemoryBarrier, (GLbitfield));

/* GL_ARB_framebuffer_no_attachments */

DECL_GLPROC(void, glFramebufferParameteri, (GLenum, GLenum, GLint));
DECL_GLPROC(void, glGetFramebufferParameteriv, (GLenum, GLenum, GLint*));

/* GL_ARB_clear_buffer_object */

DECL_GLPROC(void, glClearBufferData, (GLenum, GLenum, GLenum, GLenum, const void*));
DECL_GLPROC(void, glClearBufferSubData, (GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*));

/* GL_ARB_draw_indirect */

DECL_GLPROC(void, glDrawArraysIndirect, (GLenum, const void*));
DECL_GLPROC(void, glDrawElementsIndirect, (GLenum, GLenum, const void*));

/* GL_ARB_multi_draw_indirect */

DECL_GLPROC(void, glMultiDrawArraysIndirect, (GLenum, const void*, GLsizei, GLsizei));
DECL_GLPROC(void, glMultiDrawElementsIndirect, (GLenum, GLenum, const void*, GLsizei, GLsizei));

/* GL_ARB_direct_state_access */

DECL_GLPROC(void, glCreateTransformFeedbacks, (GLsizei, GLuint*));
DECL_GLPROC(void, glTransformFeedbackBufferBase, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glTransformFeedbackBufferRange, (GLuint, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glGetTransformFeedbackiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetTransformFeedbacki_v, (GLuint, GLenum, GLuint, GLint*));
DECL_GLPROC(void, glGetTransformFeedbacki64_v, (GLuint, GLenum, GLuint, GLint64*));
DECL_GLPROC(void, glCreateBuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glNamedBufferStorage, (GLuint, GLsizeiptr, const void*, GLbitfield));
DECL_GLPROC(void, glNamedBufferData, (GLuint, GLsizeiptr, const void*, GLenum));
DECL_GLPROC(void, glNamedBufferSubData, (GLuint, GLintptr, GLsizeiptr, const void*));
DECL_GLPROC(void, glCopyNamedBufferSubData, (GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glClearNamedBufferData, (GLuint, GLenum, GLenum, GLenum, const void*));
DECL_GLPROC(void, glClearNamedBufferSubData, (GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*));
DECL_GLPROC(void*, glMapNamedBuffer, (GLuint, GLenum));
DECL_GLPROC(void*, glMapNamedBufferRange, (GLuint, GLintptr, GLsizeiptr, GLbitfield));
DECL_GLPROC(GLboolean, glUnmapNamedBuffer, (GLuint));
DECL_GLPROC(void, glFlushMappedNamedBufferRange, (GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glGetNamedBufferParameteriv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetNamedBufferParameteri64v, (GLuint, GLenum, GLint64*));
DECL_GLPROC(void, glGetNamedBufferPointerv, (GLuint, GLenum, void**));
DECL_GLPROC(void, glGetNamedBufferSubData, (GLuint, GLintptr, GLsizeiptr, void*));
DECL_GLPROC(void, glCreateFramebuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glNamedFramebufferRenderbuffer, (GLuint, GLenum, GLenum, GLuint));
DECL_GLPROC(void, glNamedFramebufferParameteri, (GLuint, GLenum, GLint));
DECL_GLPROC(void, glNamedFramebufferTexture, (GLuint, GLenum, GLuint, GLint));
DECL_GLPROC(void, glNamedFramebufferTextureLayer, (GLuint, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(void, glNamedFramebufferDrawBuffer, (GLuint, GLenum));
DECL_GLPROC(void, glNamedFramebufferDrawBuffers, (GLuint, GLsizei, const GLenum*));
DECL_GLPROC(void, glNamedFramebufferReadBuffer, (GLuint, GLenum));
DECL_GLPROC(void, glInvalidateNamedFramebufferData, (GLuint, GLsizei, const GLenum*));
DECL_GLPROC(void, glInvalidateNamedFramebufferSubData, (GLuint, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(void, glClearNamedFramebufferiv, (GLuint, GLenum, GLint, const GLint*));
DECL_GLPROC(void, glClearNamedFramebufferuiv, (GLuint, GLenum, GLint, const GLuint*));
DECL_GLPROC(void, glClearNamedFramebufferfv, (GLuint, GLenum, GLint, const GLfloat*));
DECL_GLPROC(void, glClearNamedFramebufferfi, (GLuint, GLenum, GLint, GLfloat, GLint));
DECL_GLPROC(void, glBlitNamedFramebuffer, (GLuint, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum));
DECL_GLPROC(GLenum, glCheckNamedFramebufferStatus, (GLuint, GLenum));
DECL_GLPROC(void, glGetNamedFramebufferParameteriv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetNamedFramebufferAttachmentParameteriv, (GLuint, GLenum, GLenum, GLint*));
DECL_GLPROC(void, glCreateRenderbuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glNamedRenderbufferStorage, (GLuint, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glNamedRenderbufferStorageMultisample, (GLuint, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glGetNamedRenderbufferParameteriv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glCreateTextures, (GLenum, GLsizei, GLuint*));
DECL_GLPROC(void, glTextureBuffer, (GLuint, GLenum, GLuint));
DECL_GLPROC(void, glTextureBufferRange, (GLuint, GLenum, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(void, glTextureStorage1D, (GLuint, GLsizei, GLenum, GLsizei));
DECL_GLPROC(void, glTextureStorage2D, (GLuint, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(void, glTextureStorage3D, (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei));
DECL_GLPROC(void, glTextureStorage2DMultisample, (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(void, glTextureStorage3DMultisample, (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(void, glTextureSubImage1D, (GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(void, glTextureSubImage2D, (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(void, glTextureSubImage3D, (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(void, glCompressedTextureSubImage1D, (GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTextureSubImage2D, (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTextureSubImage3D, (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCopyTextureSubImage1D, (GLuint, GLint, GLint, GLint, GLint, GLsizei));
DECL_GLPROC(void, glCopyTextureSubImage2D, (GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(void, glCopyTextureSubImage3D, (GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(void, glTextureParameterf, (GLuint, GLenum, GLfloat));
DECL_GLPROC(void, glTextureParameterfv, (GLuint, GLenum, const GLfloat*));
DECL_GLPROC(void, glTextureParameteri, (GLuint, GLenum, GLint));
DECL_GLPROC(void, glTextureParameterIiv, (GLuint, GLenum, const GLint*));
DECL_GLPROC(void, glTextureParameterIuiv, (GLuint, GLenum, const GLuint*));
DECL_GLPROC(void, glTextureParameteriv, (GLuint, GLenum, const GLint*));
DECL_GLPROC(void, glGenerateTextureMipmap, (GLuint));
DECL_GLPROC(void, glBindTextureUnit, (GLuint, GLuint));
DECL_GLPROC(void, glGetTextureImage, (GLuint, GLint, GLenum, GLenum, GLsizei, void*));
DECL_GLPROC(void, glGetCompressedTextureImage, (GLuint, GLint, GLsizei, void*));
DECL_GLPROC(void, glGetTextureLevelParameterfv, (GLuint, GLint, GLenum, GLfloat*));
DECL_GLPROC(void, glGetTextureLevelParameteriv, (GLuint, GLint, GLenum, GLint*));
DECL_GLPROC(void, glGetTextureParameterfv, (GLuint, GLenum, GLfloat*));
DECL_GLPROC(void, glGetTextureParameterIiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetTextureParameterIuiv, (GLuint, GLenum, GLuint*));
DECL_GLPROC(void, glGetTextureParameteriv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glCreateVertexArrays, (GLsizei, GLuint*));
DECL_GLPROC(void, glDisableVertexArrayAttrib, (GLuint, GLuint));
DECL_GLPROC(void, glEnableVertexArrayAttrib, (GLuint, GLuint));
DECL_GLPROC(void, glVertexArrayElementBuffer, (GLuint, GLuint));
DECL_GLPROC(void, glVertexArrayVertexBuffer, (GLuint, GLuint, GLuint, GLintptr, GLsizei));
DECL_GLPROC(void, glVertexArrayVertexBuffers, (GLuint, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*));
DECL_GLPROC(void, glVertexArrayAttribFormat, (GLuint, GLuint, GLint, GLenum, GLboolean, GLuint));
DECL_GLPROC(void, glVertexArrayAttribIFormat, (GLuint, GLuint, GLint, GLenum, GLuint));
DECL_GLPROC(void, glVertexArrayAttribLFormat, (GLuint, GLuint, GLint, GLenum, GLuint));
DECL_GLPROC(void, glVertexArrayAttribBinding, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glVertexArrayBindingDivisor, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glGetVertexArrayiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetVertexArrayIndexediv, (GLuint, GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetVertexArrayIndexed64iv, (GLuint, GLuint, GLenum, GLint64*));
DECL_GLPROC(void, glCreateSamplers, (GLsizei, GLuint*));
DECL_GLPROC(void, glCreateProgramPipelines, (GLsizei, GLuint*));
DECL_GLPROC(void, glCreateQueries, (GLenum, GLsizei, GLuint*));
DECL_GLPROC(void, glGetQueryBufferObjectiv, (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(void, glGetQueryBufferObjectuiv, (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(void, glGetQueryBufferObjecti64v, (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(void, glGetQueryBufferObjectui64v, (GLuint, GLuint, GLenum, GLintptr));

#endif // /ifndef(__APPLE__)

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
