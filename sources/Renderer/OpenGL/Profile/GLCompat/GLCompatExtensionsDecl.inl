/*
 * GLCompatExtensionsDecl.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD


#ifndef __APPLE__

#ifndef DECL_GLPROC
#error Missing definition of macro DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS)
#endif

/* Platform specific GL extensions */

#if defined(_WIN32)

// WGL_EXT_swap_control
DECL_GLPROC(PFNWGLSWAPINTERVALEXTPROC,                              wglSwapIntervalEXT,                             BOOL,           (int));
DECL_GLPROC(PFNWGLCHOOSEPIXELFORMATARBPROC,                         wglChoosePixelFormatARB,                        BOOL,           (HDC, const int*, const FLOAT*, UINT, int*, UINT*));
DECL_GLPROC(PFNWGLCREATECONTEXTATTRIBSARBPROC,                      wglCreateContextAttribsARB,                     HGLRC,          (HDC, HGLRC, const int*));
DECL_GLPROC(PFNWGLGETEXTENSIONSSTRINGARBPROC,                       wglGetExtensionsStringARB,                      const char*,    (HDC));

#elif defined(__linux__)

// GLX_SGI_swap_control
DECL_GLPROC(PFNGLXSWAPINTERVALSGIPROC,                              glXSwapIntervalSGI,                             int,            (int));

#endif

#if defined(GL_VERSION_3_1) && !defined(GL_GLEXT_PROTOTYPES)

/* GL_ARB_compatibility */

DECL_GLPROC(PFNGLPRIMITIVERESTARTINDEXPROC,                         glPrimitiveRestartIndex,                        void,           (GLuint));

#endif

/* GL_EXT_blend_func_separate */

DECL_GLPROC(PFNGLBLENDFUNCSEPARATEPROC,                             glBlendFuncSeparate,                            void,           (GLenum, GLenum, GLenum, GLenum));

/* GL_EXT_blend_minmax */

DECL_GLPROC(PFNGLBLENDEQUATIONPROC,                                 glBlendEquation,                                void,           (GLenum));

/* GL_EXT_blend_color */

DECL_GLPROC(PFNGLBLENDCOLORPROC,                                    glBlendColor,                                   void,           (GLfloat, GLfloat, GLfloat, GLfloat));

/* GL_EXT_blend_equation_separate */

DECL_GLPROC(PFNGLBLENDEQUATIONSEPARATEPROC,                         glBlendEquationSeparate,                        void,           (GLenum, GLenum));

/* GL_ARB_draw_buffers */

DECL_GLPROC(PFNGLDRAWBUFFERSPROC,                                   glDrawBuffers,                                  void,           (GLsizei, const GLenum*));

/* GL_EXT_draw_buffers2 */

DECL_GLPROC(PFNGLCOLORMASKIPROC,                                    glColorMaski,                                   void,           (GLuint, GLboolean, GLboolean, GLboolean, GLboolean));
DECL_GLPROC(PFNGLGETBOOLEANI_VPROC,                                 glGetBooleani_v,                                void,           (GLenum, GLuint, GLboolean*));
DECL_GLPROC(PFNGLGETINTEGERI_VPROC,                                 glGetIntegeri_v,                                void,           (GLenum, GLuint, GLint*));
DECL_GLPROC(PFNGLENABLEIPROC,                                       glEnablei,                                      void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLDISABLEIPROC,                                      glDisablei,                                     void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLISENABLEDIPROC,                                    glIsEnabledi,                                   GLboolean,      (GLenum, GLuint));

/* GL_ARB_draw_buffers_blend */

DECL_GLPROC(PFNGLBLENDEQUATIONIPROC,                                glBlendEquationi,                               void,           (GLuint, GLenum));
DECL_GLPROC(PFNGLBLENDEQUATIONSEPARATEIPROC,                        glBlendEquationSeparatei,                       void,           (GLuint, GLenum, GLenum));
DECL_GLPROC(PFNGLBLENDFUNCIPROC,                                    glBlendFunci,                                   void,           (GLuint, GLenum, GLenum));
DECL_GLPROC(PFNGLBLENDFUNCSEPARATEIPROC,                            glBlendFuncSeparatei,                           void,           (GLuint, GLenum, GLenum, GLenum, GLenum));

/* GL_ARB_multitexture */

DECL_GLPROC(PFNGLACTIVETEXTUREPROC,                                 glActiveTexture,                                void,           (GLenum));

/* GL_EXT_texture3D */

DECL_GLPROC(PFNGLTEXIMAGE3DPROC,                                    glTexImage3D,                                   void,           (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLTEXSUBIMAGE3DPROC,                                 glTexSubImage3D,                                void,           (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* GL_EXT_copy_texture */

DECL_GLPROC(PFNGLCOPYTEXSUBIMAGE3DPROC,                             glCopyTexSubImage3D,                            void,           (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei));

/* GL_ARB_texture_compression */

DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE1DPROC,                          glCompressedTexImage1D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE2DPROC,                          glCompressedTexImage2D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE3DPROC,                          glCompressedTexImage3D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC,                       glCompressedTexSubImage1D,                      void,           (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC,                       glCompressedTexSubImage2D,                      void,           (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC,                       glCompressedTexSubImage3D,                      void,           (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLGETCOMPRESSEDTEXIMAGEPROC,                         glGetCompressedTexImage,                        void,           (GLenum, GLint, void*));

/* GL_ARB_vertex_buffer_object */

DECL_GLPROC(PFNGLGENBUFFERSPROC,                                    glGenBuffers,                                   void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETEBUFFERSPROC,                                 glDeleteBuffers,                                void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDBUFFERPROC,                                    glBindBuffer,                                   void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLISBUFFERPROC,                                      glIsBuffer,                                     GLboolean,      (GLuint));
DECL_GLPROC(PFNGLBUFFERDATAPROC,                                    glBufferData,                                   void,           (GLenum, GLsizeiptr, const void*, GLenum));
DECL_GLPROC(PFNGLBUFFERSUBDATAPROC,                                 glBufferSubData,                                void,           (GLenum, GLintptr, GLsizeiptr, const void*));
DECL_GLPROC(PFNGLGETBUFFERSUBDATAPROC,                              glGetBufferSubData,                             void,           (GLenum, GLintptr, GLsizeiptr, void*));
DECL_GLPROC(PFNGLMAPBUFFERPROC,                                     glMapBuffer,                                    void*,          (GLenum, GLenum));
DECL_GLPROC(PFNGLUNMAPBUFFERPROC,                                   glUnmapBuffer,                                  GLboolean,      (GLenum));
DECL_GLPROC(PFNGLGETBUFFERPARAMETERIVPROC,                          glGetBufferParameteriv,                         void,           (GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLGETBUFFERPOINTERVPROC,                             glGetBufferPointerv,                            void,           (GLenum, GLenum, void**));

/* GL_ARB_map_buffer_range */

DECL_GLPROC(PFNGLMAPBUFFERRANGEPROC,                                glMapBufferRange,                               void*,          (GLenum, GLintptr, GLsizeiptr, GLbitfield));
DECL_GLPROC(PFNGLFLUSHMAPPEDBUFFERRANGEPROC,                        glFlushMappedBufferRange,                       void,           (GLenum, GLintptr, GLsizeiptr));

/* GL_ARB_vertex_shader */

DECL_GLPROC(PFNGLENABLEVERTEXATTRIBARRAYPROC,                       glEnableVertexAttribArray,                      void,           (GLuint));
DECL_GLPROC(PFNGLDISABLEVERTEXATTRIBARRAYPROC,                      glDisableVertexAttribArray,                     void,           (GLuint));
DECL_GLPROC(PFNGLVERTEXATTRIBPOINTERPROC,                           glVertexAttribPointer,                          void,           (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*));
DECL_GLPROC(PFNGLBINDATTRIBLOCATIONPROC,                            glBindAttribLocation,                           void,           (GLuint, GLuint, const GLchar*));

/* GL_EXT_framebuffer_object */

DECL_GLPROC(PFNGLGENRENDERBUFFERSEXTPROC,                           glGenRenderbuffersEXT,                          void,           (GLsizei n, GLuint *));
DECL_GLPROC(PFNGLDELETERENDERBUFFERSEXTPROC,                        glDeleteRenderbuffersEXT,                       void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDRENDERBUFFEREXTPROC,                           glBindRenderbufferEXT,                          void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLRENDERBUFFERSTORAGEEXTPROC,                        glRenderbufferStorageEXT,                       void,           (GLenum, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC,             glRenderbufferStorageMultisampleEXT,            void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC,                 glGetRenderbufferParameterivEXT,                void,           (GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLGENFRAMEBUFFERSEXTPROC,                            glGenFramebuffersEXT,                           void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETEFRAMEBUFFERSEXTPROC,                         glDeleteFramebuffersEXT,                        void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDFRAMEBUFFEREXTPROC,                            glBindFramebufferEXT,                           void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC,                     glCheckFramebufferStatusEXT,                    GLenum,         (GLenum));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE1DEXTPROC,                       glFramebufferTexture1DEXT,                      void,           (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,                       glFramebufferTexture2DEXT,                      void,           (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE3DEXTPROC,                       glFramebufferTexture3DEXT,                      void,           (GLenum, GLenum, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC,                    glFramebufferRenderbufferEXT,                   void,           (GLenum, GLenum, GLenum, GLuint));
DECL_GLPROC(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC,        glGetFramebufferAttachmentParameterivEXT,       void,           (GLenum, GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLBLITFRAMEBUFFEREXTPROC,                            glBlitFramebufferEXT,                           void,           (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum));
DECL_GLPROC(PFNGLGENERATEMIPMAPEXTPROC,                             glGenerateMipmapEXT,                            void,           (GLenum));

/* GL_ARB_shader_objects */

DECL_GLPROC(PFNGLCREATESHADERPROC,                                  glCreateShader,                                 GLuint,         (GLenum));
DECL_GLPROC(PFNGLSHADERSOURCEPROC,                                  glShaderSource,                                 void,           (GLuint, GLsizei, const GLchar* const*, const GLint*));
DECL_GLPROC(PFNGLCOMPILESHADERPROC,                                 glCompileShader,                                void,           (GLuint));
DECL_GLPROC(PFNGLGETSHADERIVPROC,                                   glGetShaderiv,                                  void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETSHADERINFOLOGPROC,                              glGetShaderInfoLog,                             void,           (GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLGETSHADERSOURCEPROC,                               glGetShaderSource,                              void,           (GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLDELETESHADERPROC,                                  glDeleteShader,                                 void,           (GLuint));
DECL_GLPROC(PFNGLCREATEPROGRAMPROC,                                 glCreateProgram,                                GLuint,         (void));
DECL_GLPROC(PFNGLDELETEPROGRAMPROC,                                 glDeleteProgram,                                void,           (GLuint));
DECL_GLPROC(PFNGLATTACHSHADERPROC,                                  glAttachShader,                                 void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLDETACHSHADERPROC,                                  glDetachShader,                                 void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLLINKPROGRAMPROC,                                   glLinkProgram,                                  void,           (GLuint));
DECL_GLPROC(PFNGLVALIDATEPROGRAMPROC,                               glValidateProgram,                              void,           (GLuint));
DECL_GLPROC(PFNGLGETPROGRAMIVPROC,                                  glGetProgramiv,                                 void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETPROGRAMINFOLOGPROC,                             glGetProgramInfoLog,                            void,           (GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLUSEPROGRAMPROC,                                    glUseProgram,                                   void,           (GLuint));
DECL_GLPROC(PFNGLGETACTIVEATTRIBPROC,                               glGetActiveAttrib,                              void,           (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*));
DECL_GLPROC(PFNGLGETATTRIBLOCATIONPROC,                             glGetAttribLocation,                            GLint,          (GLuint, const GLchar*));
DECL_GLPROC(PFNGLGETACTIVEUNIFORMARBPROC,                           glGetActiveUniform,                             void,           (GLhandleARB, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLcharARB*));
DECL_GLPROC(PFNGLGETUNIFORMLOCATIONPROC,                            glGetUniformLocation,                           GLint,          (GLuint, const GLchar*));
DECL_GLPROC(PFNGLGETATTACHEDSHADERSPROC,                            glGetAttachedShaders,                           void,           (GLuint, GLsizei, GLsizei*, GLuint*));
DECL_GLPROC(PFNGLUNIFORM1FPROC,                                     glUniform1f,                                    void,           (GLint, GLfloat));
DECL_GLPROC(PFNGLUNIFORM2FPROC,                                     glUniform2f,                                    void,           (GLint, GLfloat, GLfloat));
DECL_GLPROC(PFNGLUNIFORM3FPROC,                                     glUniform3f,                                    void,           (GLint, GLfloat, GLfloat, GLfloat));
DECL_GLPROC(PFNGLUNIFORM4FPROC,                                     glUniform4f,                                    void,           (GLint, GLfloat, GLfloat, GLfloat, GLfloat));
DECL_GLPROC(PFNGLUNIFORM1IPROC,                                     glUniform1i,                                    void,           (GLint, GLint));
DECL_GLPROC(PFNGLUNIFORM2IPROC,                                     glUniform2i,                                    void,           (GLint, GLint, GLint));
DECL_GLPROC(PFNGLUNIFORM3IPROC,                                     glUniform3i,                                    void,           (GLint, GLint, GLint, GLint));
DECL_GLPROC(PFNGLUNIFORM4IPROC,                                     glUniform4i,                                    void,           (GLint, GLint, GLint, GLint, GLint));
DECL_GLPROC(PFNGLUNIFORM1FVPROC,                                    glUniform1fv,                                   void,           (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORM2FVPROC,                                    glUniform2fv,                                   void,           (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORM3FVPROC,                                    glUniform3fv,                                   void,           (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORM4FVPROC,                                    glUniform4fv,                                   void,           (GLint, GLsizei, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORM1IVPROC,                                    glUniform1iv,                                   void,           (GLint, GLsizei, const GLint*));
DECL_GLPROC(PFNGLUNIFORM2IVPROC,                                    glUniform2iv,                                   void,           (GLint, GLsizei, const GLint*));
DECL_GLPROC(PFNGLUNIFORM3IVPROC,                                    glUniform3iv,                                   void,           (GLint, GLsizei, const GLint*));
DECL_GLPROC(PFNGLUNIFORM4IVPROC,                                    glUniform4iv,                                   void,           (GLint, GLsizei, const GLint*));
DECL_GLPROC(PFNGLUNIFORMMATRIX2FVPROC,                              glUniformMatrix2fv,                             void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3FVPROC,                              glUniformMatrix3fv,                             void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4FVPROC,                              glUniformMatrix4fv,                             void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLGETUNIFORMIVPROC,                                  glGetUniformiv,                                 void,           (GLuint, GLint, GLint*));
DECL_GLPROC(PFNGLGETUNIFORMFVPROC,                                  glGetUniformfv,                                 void,           (GLuint, GLint, GLfloat*));

/* **GL_ARB_shader_objects** TODO: which extension from GL 2.1 ??? */

DECL_GLPROC(PFNGLUNIFORMMATRIX2X3FVPROC,                            glUniformMatrix2x3fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX2X4FVPROC,                            glUniformMatrix2x4fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3X2FVPROC,                            glUniformMatrix3x2fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3X4FVPROC,                            glUniformMatrix3x4fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4X2FVPROC,                            glUniformMatrix4x2fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4X3FVPROC,                            glUniformMatrix4x3fv,                           void,           (GLint, GLsizei, GLboolean, const GLfloat*));

/* GL_ARB_occlusion_query */

DECL_GLPROC(PFNGLGENQUERIESPROC,                                    glGenQueries,                                   void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETEQUERIESPROC,                                 glDeleteQueries,                                void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBEGINQUERYPROC,                                    glBeginQuery,                                   void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLENDQUERYPROC,                                      glEndQuery,                                     void,           (GLenum));
DECL_GLPROC(PFNGLGETQUERYOBJECTIVPROC,                              glGetQueryObjectiv,                             void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETQUERYOBJECTUIVPROC,                             glGetQueryObjectuiv,                            void,           (GLuint, GLenum, GLuint*));

/* GL_NV_conditional_render */

DECL_GLPROC(PFNGLBEGINCONDITIONALRENDERPROC,                        glBeginConditionalRender,                       void,           (GLuint, GLenum));
DECL_GLPROC(PFNGLENDCONDITIONALRENDERPROC,                          glEndConditionalRender,                         void,           (void));

/* GL_ARB_timer_query */

DECL_GLPROC(PFNGLQUERYCOUNTERPROC,                                  glQueryCounter,                                 void,           (GLuint, GLenum));
DECL_GLPROC(PFNGLGETQUERYOBJECTI64VPROC,                            glGetQueryObjecti64v,                           void,           (GLuint, GLenum, GLint64*));
DECL_GLPROC(PFNGLGETQUERYOBJECTUI64VPROC,                           glGetQueryObjectui64v,                          void,           (GLuint, GLenum, GLuint64*));

/* GL_ATI_separate_stencil ??? */

DECL_GLPROC(PFNGLSTENCILFUNCSEPARATEPROC,                           glStencilFuncSeparate,                          void,           (GLenum, GLenum, GLint, GLuint));
DECL_GLPROC(PFNGLSTENCILMASKSEPARATEPROC,                           glStencilMaskSeparate,                          void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLSTENCILOPSEPARATEPROC,                             glStencilOpSeparate,                            void,           (GLenum, GLenum, GLenum, GLenum));

/* GL_EXT_transform_feedback */

DECL_GLPROC(PFNGLBINDBUFFERRANGEPROC,                               glBindBufferRange,                              void,           (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLBEGINTRANSFORMFEEDBACKPROC,                        glBeginTransformFeedback,                       void,           (GLenum));
DECL_GLPROC(PFNGLENDTRANSFORMFEEDBACKPROC,                          glEndTransformFeedback,                         void,           (void));
DECL_GLPROC(PFNGLTRANSFORMFEEDBACKVARYINGSPROC,                     glTransformFeedbackVaryings,                    void,           (GLuint, GLsizei, const GLchar *const*, GLenum));
DECL_GLPROC(PFNGLGETTRANSFORMFEEDBACKVARYINGPROC,                   glGetTransformFeedbackVarying,                  void,           (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*));

/* GL_NV_transform_feedback */

DECL_GLPROC(PFNGLBINDBUFFERRANGENVPROC,                             glBindBufferRangeNV,                            void,           (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLBEGINTRANSFORMFEEDBACKNVPROC,                      glBeginTransformFeedbackNV,                     void,           (GLenum));
DECL_GLPROC(PFNGLENDTRANSFORMFEEDBACKNVPROC,                        glEndTransformFeedbackNV,                       void,           (void));
DECL_GLPROC(PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC,                   glTransformFeedbackVaryingsNV,                  void,           (GLuint, GLsizei, const GLint*, GLenum));
DECL_GLPROC(PFNGLGETVARYINGLOCATIONNVPROC,                          glGetVaryingLocationNV,                         GLint,          (GLuint, const GLchar*));
DECL_GLPROC(PFNGLGETACTIVEVARYINGNVPROC,                            glGetActiveVaryingNV,                           void,           (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*));

/* GL_ARB_sync */

DECL_GLPROC(PFNGLFENCESYNCPROC,                                     glFenceSync,                                    GLsync,         (GLenum, GLbitfield));
DECL_GLPROC(PFNGLISSYNCPROC,                                        glIsSync,                                       GLboolean,      (GLsync));
DECL_GLPROC(PFNGLDELETESYNCPROC,                                    glDeleteSync,                                   void,           (GLsync));
DECL_GLPROC(PFNGLCLIENTWAITSYNCPROC,                                glClientWaitSync,                               GLenum,         (GLsync, GLbitfield, GLuint64));
DECL_GLPROC(PFNGLWAITSYNCPROC,                                      glWaitSync,                                     void,           (GLsync, GLbitfield, GLuint64));
DECL_GLPROC(PFNGLGETINTEGER64VPROC,                                 glGetInteger64v,                                void,           (GLenum pname, GLint64*));
DECL_GLPROC(PFNGLGETSYNCIVPROC,                                     glGetSynciv,                                    void,           (GLsync, GLenum, GLsizei, GLsizei*, GLint*));

/* GL_ARB_internalformat_query */

DECL_GLPROC(PFNGLGETINTERNALFORMATIVPROC,                           glGetInternalformativ,                          void,           (GLenum, GLenum, GLenum, GLsizei, GLint*));

/* GL_ARB_internalformat_query2 */

DECL_GLPROC(PFNGLGETINTERNALFORMATI64VPROC,                         glGetInternalformati64v,                        void,           (GLenum, GLenum, GLenum, GLsizei, GLint64*));

/* GL_ARB_ES2_compatibility */

DECL_GLPROC(PFNGLRELEASESHADERCOMPILERPROC,                         glReleaseShaderCompiler,                        void,           (void));
DECL_GLPROC(PFNGLSHADERBINARYPROC,                                  glShaderBinary,                                 void,           (GLsizei, const GLuint*, GLenum, const void*, GLsizei));
DECL_GLPROC(PFNGLGETSHADERPRECISIONFORMATPROC,                      glGetShaderPrecisionFormat,                     void,           (GLenum, GLenum, GLint*, GLint*));
DECL_GLPROC(PFNGLDEPTHRANGEFPROC,                                   glDepthRangef,                                  void,           (GLclampf, GLclampf));
DECL_GLPROC(PFNGLCLEARDEPTHFPROC,                                   glClearDepthf,                                  void,           (GLclampf));

/* GL_ARB_buffer_storage */

DECL_GLPROC(PFNGLBUFFERSTORAGEPROC,                                 glBufferStorage,                                void,           (GLenum, GLsizeiptr, const void*, GLbitfield));

/* GL_ARB_copy_buffer */

DECL_GLPROC(PFNGLCOPYBUFFERSUBDATAPROC,                             glCopyBufferSubData,                            void,           (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr));

/* GL_ARB_copy_image */

DECL_GLPROC(PFNGLCOPYIMAGESUBDATAPROC,                              glCopyImageSubData,                             void,           (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei));

/* GL_ARB_polygon_offset_clamp */

DECL_GLPROC(PFNGLPOLYGONOFFSETCLAMPPROC,                            glPolygonOffsetClamp,                           void,           (GLfloat, GLfloat, GLfloat));

/* GL_ARB_clear_buffer_object */

DECL_GLPROC(PFNGLCLEARBUFFERDATAPROC,                               glClearBufferData,                              void,           (GLenum, GLenum, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLCLEARBUFFERSUBDATAPROC,                            glClearBufferSubData,                           void,           (GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*));

#undef DECL_GLPROC

#endif // /__APPLE__



// ================================================================================
