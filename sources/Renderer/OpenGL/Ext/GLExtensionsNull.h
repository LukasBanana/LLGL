/*
 * GLExtensionNull.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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


#include <stdexcept>
#include "../OpenGL.h"


namespace LLGL
{


#ifdef __LLGL_DEF_GL_DUMMY_PROCS__

#define DECL_GLPROC(RTYPE, NAME, ARGS)                                                              \
    RTYPE APIENTRY Dummy_##NAME ARGS                                                                \
    {                                                                                               \
        throw std::runtime_error(                                                                   \
            "illegal use of unsupported OpenGL extension procedure: \"" + std::string(#NAME) + "\"" \
        );                                                                                          \
    }

#else

#define DECL_GLPROC(RTYPE, NAME, ARGS) \
    RTYPE APIENTRY Dummy_##NAME ARGS

#endif

#ifndef __APPLE__

/* --- Platform specific GL extensions --- */

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

/* --- GL 3.0 extensions (for Core Profile) --- */

DECL_GLPROC(const GLubyte*, glGetStringi, (GLenum GLuint));

#endif

/* --- Blending (GL_ARB_draw_buffers_blend) --- */

DECL_GLPROC(void, glBlendFuncSeparate, (GLenum, GLenum, GLenum, GLenum));
DECL_GLPROC(void, glBlendFuncSeparatei, (GLuint, GLenum, GLenum, GLenum, GLenum));

/* --- Indexed (GL_EXT_draw_buffers2) --- */

DECL_GLPROC(void, glColorMaski, (GLuint, GLboolean, GLboolean, GLboolean, GLboolean));
DECL_GLPROC(void, glGetBooleani_v, (GLenum, GLuint, GLboolean*));
DECL_GLPROC(void, glGetIntegeri_v, (GLenum, GLuint, GLint*));
DECL_GLPROC(void, glEnablei, (GLenum, GLuint));
DECL_GLPROC(void, glDisablei, (GLenum, GLuint));
DECL_GLPROC(GLboolean, glIsEnabledi, (GLenum, GLuint));

/* --- Multi texture (GL_ARB_multitexture) --- */

DECL_GLPROC(void, glActiveTexture, (GLenum));

/* --- 3D texture (GL_EXT_texture3D) --- */

DECL_GLPROC(void, glTexImage3D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(void, glTexSubImage3D, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* --- Clear texture (GL_ARB_clear_texture) --- */

DECL_GLPROC(void, glClearTexImage, (GLuint, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(void, glClearTexSubImage, (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* --- Texture compression (GL_ARB_texture_compression) --- */

DECL_GLPROC(void, glCompressedTexImage1D, (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexImage2D, (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexImage3D, (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage1D, (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glCompressedTexSubImage3D, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glGetCompressedTexImage, (GLenum, GLint, void*));

/* --- Sampler objects (GL_ARB_sampler_objects) --- */

DECL_GLPROC(void, glGenSamplers, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteSamplers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindSampler, (GLuint, GLuint));
DECL_GLPROC(void, glSamplerParameteri, (GLuint, GLenum, GLint));
DECL_GLPROC(void, glSamplerParameterf, (GLuint, GLenum, GLfloat));
DECL_GLPROC(void, glSamplerParameteriv, (GLuint, GLenum, const GLint*));
DECL_GLPROC(void, glSamplerParameterfv, (GLuint, GLenum, const GLfloat*));

/* --- Multi bind (GL_ARB_multi_bind) --- */

DECL_GLPROC(void, glBindBuffersBase, (GLenum, GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindBuffersRange, (GLenum, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizeiptr*));
DECL_GLPROC(void, glBindTextures, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindSamplers, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindImageTextures, (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(void, glBindVertexBuffers, (GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*));

/* --- Vertex buffer object (GL_ARB_vertex_buffer_object) --- */

DECL_GLPROC(void, glGenBuffers, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteBuffers, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindBuffer, (GLenum, GLuint));
DECL_GLPROC(void, glBufferData, (GLenum, GLsizeiptr, const void*, GLenum));
DECL_GLPROC(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const void*));
DECL_GLPROC(void*, glMapBuffer, (GLenum, GLenum));
DECL_GLPROC(GLboolean, glUnmapBuffer, (GLenum));

/* --- Vertex attributes (GL_ARB_vertex_buffer_object???) --- */

DECL_GLPROC(void, glEnableVertexAttribArray, (GLuint));
DECL_GLPROC(void, glDisableVertexAttribArray, (GLuint));
DECL_GLPROC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*));
DECL_GLPROC(void, glVertexAttribIPointer, (GLuint, GLint, GLenum, GLsizei, const void*));
DECL_GLPROC(void, glBindAttribLocation, (GLuint, GLuint, const GLchar*));

/* --- Draw buffers (GL_ARB_draw_buffers) --- */

DECL_GLPROC(void, glDrawBuffers, (GLsizei, const GLenum*));

/* --- Vertex array objects (GL_ARB_vertex_array_object) --- */

DECL_GLPROC(void, glGenVertexArrays, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteVertexArrays, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBindVertexArray, (GLuint));

/* --- Frame buffer objects (GL_ARB_framebuffer_object) --- */

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

/* --- Instanced drawing (GL_ARB_draw_instanced) --- */

DECL_GLPROC(void, glDrawArraysInstanced, (GLenum, GLint, GLsizei, GLsizei));
DECL_GLPROC(void, glDrawElementsInstanced, (GLenum, GLsizei, GLenum, const void*, GLsizei));

/* --- Base vertex drawing (GL_ARB_draw_elements_base_vertex) --- */

DECL_GLPROC(void, glDrawElementsBaseVertex, (GLenum, GLsizei, GLenum, const void*, GLint));
DECL_GLPROC(void, glDrawElementsInstancedBaseVertex, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint));

/* --- Instanced offset drawing (GL_ARB_base_instance) --- */

DECL_GLPROC(void, glDrawArraysInstancedBaseInstance, (GLenum, GLint, GLsizei, GLsizei, GLuint));
DECL_GLPROC(void, glDrawElementsInstancedBaseInstance, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLuint));
DECL_GLPROC(void, glDrawElementsInstancedBaseVertexBaseInstance, (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint, GLuint));

/* --- OpenGL shader extension (GL_ARB_shader_objects) --- */

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

/* --- Tessellation shader (GL_ARB_tessellation_shader) --- */

DECL_GLPROC(void, glPatchParameteri, (GLenum, GLint));
DECL_GLPROC(void, glPatchParameterfv, (GLenum, const GLfloat*));

/* --- Compute shader (GL_ARB_compute_shader) --- */

DECL_GLPROC(void, glDispatchCompute, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glDispatchComputeIndirect, (GLintptr));

/* --- Binary program (GL_ARB_get_program_binary) --- */

DECL_GLPROC(void, glGetProgramBinary, (GLuint, GLsizei, GLsizei*, GLenum*, void*));
DECL_GLPROC(void, glProgramBinary, (GLuint, GLenum, const void*, GLsizei));
DECL_GLPROC(void, glProgramParameteri, (GLuint, GLenum, GLint));

/* --- Program interface query (GL_ARB_program_interface_query) --- */

DECL_GLPROC(void, glGetProgramInterfaceiv, (GLuint, GLenum, GLenum, GLint*));
DECL_GLPROC(GLuint, glGetProgramResourceIndex, (GLuint, GLenum, const GLchar*));
DECL_GLPROC(void, glGetProgramResourceName, (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glGetProgramResourceiv, (GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLint*));
DECL_GLPROC(GLint, glGetProgramResourceLocation, (GLuint, GLenum, const GLchar*));
DECL_GLPROC(GLint, glGetProgramResourceLocationIndex, (GLuint, GLenum, const GLchar*));

/* --- Uniform buffer objects (GL_ARB_uniform_buffer_objects) --- */

DECL_GLPROC(GLuint, glGetUniformBlockIndex, (GLuint, const GLchar*));
DECL_GLPROC(void, glGetActiveUniformBlockiv, (GLuint, GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetActiveUniformBlockName, (GLuint, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(void, glUniformBlockBinding, (GLuint, GLuint, GLuint));
DECL_GLPROC(void, glBindBufferBase, (GLenum, GLuint, GLuint));

/* --- Shader storage buffer objects (GL_ARB_shader_storage_buffer_object) --- */

DECL_GLPROC(void, glShaderStorageBlockBinding, (GLuint, GLuint, GLuint));

/* --- Query objects (GL_ARB_occlusion_query) --- */

DECL_GLPROC(void, glGenQueries, (GLsizei, GLuint*));
DECL_GLPROC(void, glDeleteQueries, (GLsizei, const GLuint*));
DECL_GLPROC(void, glBeginQuery, (GLenum, GLuint));
DECL_GLPROC(void, glEndQuery, (GLenum));
DECL_GLPROC(void, glGetQueryObjectiv, (GLuint, GLenum, GLint*));
DECL_GLPROC(void, glGetQueryObjectuiv, (GLuint, GLenum, GLuint*));

/* --- Condition render objects (GL_NV_conditional_render) --- */

DECL_GLPROC(void, glBeginConditionalRender, (GLuint, GLenum));
DECL_GLPROC(void, glEndConditionalRender, (void));

/* --- Timer query objects (GL_ARB_timer_query) --- */

DECL_GLPROC(void, glQueryCounter, (GLuint, GLenum));
DECL_GLPROC(void, glGetQueryObjecti64v, (GLuint, GLenum, GLint64*));
DECL_GLPROC(void, glGetQueryObjectui64v, (GLuint, GLenum, GLuint64*));

/* --- Viewport array (GL_ARB_viewport_array) --- */

DECL_GLPROC(void, glViewportArrayv, (GLuint, GLsizei, const GLfloat*));
DECL_GLPROC(void, glScissorArrayv, (GLuint, GLsizei, const GLint*));
DECL_GLPROC(void, glDepthRangeArrayv, (GLuint, GLsizei, const GLdouble*));

/* --- ??? --- */

DECL_GLPROC(void, glStencilFuncSeparate, (GLenum, GLenum, GLint, GLuint));
DECL_GLPROC(void, glStencilMaskSeparate, (GLenum, GLuint));
DECL_GLPROC(void, glStencilOpSeparate, (GLenum, GLenum, GLenum, GLenum));

/* --- Debug context (GL_KHR_debug) --- */

DECL_GLPROC(void, glDebugMessageCallback, (GLDEBUGPROC, const void*));

/* --- Clipping control (GL_ARB_clip_control) --- */

DECL_GLPROC(void, glClipControl, (GLenum, GLenum));

#endif // /ifndef(__APPLE__)

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
