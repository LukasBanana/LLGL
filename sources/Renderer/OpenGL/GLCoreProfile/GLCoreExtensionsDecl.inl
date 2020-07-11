/*
 * GLCoreExtensionsDecl.inl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD

/*
All OpenGL extension functions are declared here.
Depending on the following macros being defined, the respective implementation is enabled:
- LLGL_DEF_GL_PROXY_PROCS: defines the proxy functions for potentially unsupported GL extensions
- LLGL_DECL_GL_PROXY_PROCS: declares the proxy functions for potentially unsupported GL extensions
- LLGL_DEF_GL_EXT_PROCS: defines the global function pointer for GL extensions
- None: declares the global function pointer for GL extensions
*/


#ifndef __APPLE__

#if defined LLGL_DEF_GL_PROXY_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS            \
    {                                           \
        ErrUnsupportedGLProc(#NAME);            \
    }

#elif defined LLGL_DECL_GL_PROXY_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS

#elif defined LLGL_DEF_GL_EXT_PROCS

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = nullptr

#else

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    extern PFNTYPE NAME

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

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

/* GL 3.0 extensions (for Core Profile) */

DECL_GLPROC(PFNGLGETSTRINGIPROC,                                    glGetStringi,                                   const GLubyte*, (GLenum GLuint));

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

/* GL_ARB_clear_texture */

DECL_GLPROC(PFNGLCLEARTEXIMAGEPROC,                                 glClearTexImage,                                void,           (GLuint, GLint, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLCLEARTEXSUBIMAGEPROC,                              glClearTexSubImage,                             void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));

/* GL_ARB_texture_compression */

DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE1DPROC,                          glCompressedTexImage1D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE2DPROC,                          glCompressedTexImage2D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXIMAGE3DPROC,                          glCompressedTexImage3D,                         void,           (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC,                       glCompressedTexSubImage1D,                      void,           (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC,                       glCompressedTexSubImage2D,                      void,           (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC,                       glCompressedTexSubImage3D,                      void,           (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLGETCOMPRESSEDTEXIMAGEPROC,                         glGetCompressedTexImage,                        void,           (GLenum, GLint, void*));

/* GL_ARB_texture_multisample */

DECL_GLPROC(PFNGLTEXIMAGE2DMULTISAMPLEPROC,                         glTexImage2DMultisample,                        void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(PFNGLTEXIMAGE3DMULTISAMPLEPROC,                         glTexImage3DMultisample,                        void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(PFNGLGETMULTISAMPLEFVPROC,                              glGetMultisamplefv,                             void,           (GLenum, GLuint, GLfloat*));
DECL_GLPROC(PFNGLSAMPLEMASKIPROC,                                   glSampleMaski,                                  void,           (GLuint, GLbitfield));

/* GL_ARB_texture_view */

DECL_GLPROC(PFNGLTEXTUREVIEWPROC,                                   glTextureView,                                  void,           (GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint));

/* GL_ARB_sampler_objects */

DECL_GLPROC(PFNGLGENSAMPLERSPROC,                                   glGenSamplers,                                  void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETESAMPLERSPROC,                                glDeleteSamplers,                               void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDSAMPLERPROC,                                   glBindSampler,                                  void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLSAMPLERPARAMETERIPROC,                             glSamplerParameteri,                            void,           (GLuint, GLenum, GLint));
DECL_GLPROC(PFNGLSAMPLERPARAMETERFPROC,                             glSamplerParameterf,                            void,           (GLuint, GLenum, GLfloat));
DECL_GLPROC(PFNGLSAMPLERPARAMETERIVPROC,                            glSamplerParameteriv,                           void,           (GLuint, GLenum, const GLint*));
DECL_GLPROC(PFNGLSAMPLERPARAMETERFVPROC,                            glSamplerParameterfv,                           void,           (GLuint, GLenum, const GLfloat*));

/* GL_ARB_multi_bind */

DECL_GLPROC(PFNGLBINDBUFFERSBASEPROC,                               glBindBuffersBase,                              void,           (GLenum, GLuint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDBUFFERSRANGEPROC,                              glBindBuffersRange,                             void,           (GLenum, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizeiptr*));
DECL_GLPROC(PFNGLBINDTEXTURESPROC,                                  glBindTextures,                                 void,           (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDSAMPLERSPROC,                                  glBindSamplers,                                 void,           (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDIMAGETEXTURESPROC,                             glBindImageTextures,                            void,           (GLuint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDVERTEXBUFFERSPROC,                             glBindVertexBuffers,                            void,           (GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*));

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

/* GL_ARB_vertex_shader */

DECL_GLPROC(PFNGLENABLEVERTEXATTRIBARRAYPROC,                       glEnableVertexAttribArray,                      void,           (GLuint));
DECL_GLPROC(PFNGLDISABLEVERTEXATTRIBARRAYPROC,                      glDisableVertexAttribArray,                     void,           (GLuint));
DECL_GLPROC(PFNGLVERTEXATTRIBPOINTERPROC,                           glVertexAttribPointer,                          void,           (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*));
DECL_GLPROC(PFNGLBINDATTRIBLOCATIONPROC,                            glBindAttribLocation,                           void,           (GLuint, GLuint, const GLchar*));

/* GL_EXT_gpu_shader4 */

DECL_GLPROC(PFNGLVERTEXATTRIBIPOINTERPROC,                          glVertexAttribIPointer,                         void,           (GLuint, GLint, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLBINDFRAGDATALOCATIONPROC,                          glBindFragDataLocation,                         void,           (GLuint, GLuint, const GLchar*));
DECL_GLPROC(PFNGLGETFRAGDATALOCATIONPROC,                           glGetFragDataLocation,                          GLint,          (GLuint, const GLchar*));

/* GL_ARB_instanced_arrays */

DECL_GLPROC(PFNGLVERTEXATTRIBDIVISORPROC,                           glVertexAttribDivisor,                          void,           (GLuint, GLuint));

/* GL_ARB_vertex_array_object */

DECL_GLPROC(PFNGLGENVERTEXARRAYSPROC,                               glGenVertexArrays,                              void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETEVERTEXARRAYSPROC,                            glDeleteVertexArrays,                           void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDVERTEXARRAYPROC,                               glBindVertexArray,                              void,           (GLuint));
DECL_GLPROC(PFNGLISVERTEXARRAYPROC,                                 glIsVertexArray,                                GLboolean,      (GLuint));

/* GL_ARB_framebuffer_object */

DECL_GLPROC(PFNGLGENRENDERBUFFERSPROC,                              glGenRenderbuffers,                             void,           (GLsizei n, GLuint *));
DECL_GLPROC(PFNGLDELETERENDERBUFFERSPROC,                           glDeleteRenderbuffers,                          void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDRENDERBUFFERPROC,                              glBindRenderbuffer,                             void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLRENDERBUFFERSTORAGEPROC,                           glRenderbufferStorage,                          void,           (GLenum, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC,                glRenderbufferStorageMultisample,               void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLGETRENDERBUFFERPARAMETERIVPROC,                    glGetRenderbufferParameteriv,                   void,           (GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLGENFRAMEBUFFERSPROC,                               glGenFramebuffers,                              void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDELETEFRAMEBUFFERSPROC,                            glDeleteFramebuffers,                           void,           (GLsizei, const GLuint*));
DECL_GLPROC(PFNGLBINDFRAMEBUFFERPROC,                               glBindFramebuffer,                              void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC,                        glCheckFramebufferStatus,                       GLenum,         (GLenum));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTUREPROC,                            glFramebufferTexture,                           void,           (GLenum, GLenum, GLuint, GLint)); // <--- other extension!
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE1DPROC,                          glFramebufferTexture1D,                         void,           (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE2DPROC,                          glFramebufferTexture2D,                         void,           (GLenum, GLenum, GLenum, GLuint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURE3DPROC,                          glFramebufferTexture3D,                         void,           (GLenum, GLenum, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERTEXTURELAYERPROC,                       glFramebufferTextureLayer,                      void,           (GLenum, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC,                       glFramebufferRenderbuffer,                      void,           (GLenum, GLenum, GLenum, GLuint));
DECL_GLPROC(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC,           glGetFramebufferAttachmentParameteriv,          void,           (GLenum, GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLBLITFRAMEBUFFERPROC,                               glBlitFramebuffer,                              void,           (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum));
DECL_GLPROC(PFNGLGENERATEMIPMAPPROC,                                glGenerateMipmap,                               void,           (GLenum));

#if 1 //WHICH EXTENSION???
DECL_GLPROC(PFNGLCLEARBUFFERIVPROC,                                 glClearBufferiv,                                void,           (GLenum, GLint, const GLint*));
DECL_GLPROC(PFNGLCLEARBUFFERUIVPROC,                                glClearBufferuiv,                               void,           (GLenum, GLint, const GLuint*));
DECL_GLPROC(PFNGLCLEARBUFFERFVPROC,                                 glClearBufferfv,                                void,           (GLenum, GLint, const GLfloat*));
DECL_GLPROC(PFNGLCLEARBUFFERFIPROC,                                 glClearBufferfi,                                void,           (GLenum, GLint, GLfloat, GLint));
#endif

/* GL_ARB_draw_instanced */

DECL_GLPROC(PFNGLDRAWARRAYSINSTANCEDPROC,                           glDrawArraysInstanced,                          void,           (GLenum, GLint, GLsizei, GLsizei));
DECL_GLPROC(PFNGLDRAWELEMENTSINSTANCEDPROC,                         glDrawElementsInstanced,                        void,           (GLenum, GLsizei, GLenum, const void*, GLsizei));

/* GL_ARB_draw_elements_base_vertex */

DECL_GLPROC(PFNGLDRAWELEMENTSBASEVERTEXPROC,                        glDrawElementsBaseVertex,                       void,           (GLenum, GLsizei, GLenum, const void*, GLint));
DECL_GLPROC(PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC,               glDrawElementsInstancedBaseVertex,              void,           (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint));

/* GL_ARB_base_instance */

DECL_GLPROC(PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC,               glDrawArraysInstancedBaseInstance,              void,           (GLenum, GLint, GLsizei, GLsizei, GLuint));
DECL_GLPROC(PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC,             glDrawElementsInstancedBaseInstance,            void,           (GLenum, GLsizei, GLenum, const void*, GLsizei, GLuint));
DECL_GLPROC(PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC,   glDrawElementsInstancedBaseVertexBaseInstance,  void,           (GLenum, GLsizei, GLenum, const void*, GLsizei, GLint, GLuint));

/* GL_ARB_shader_objects */

DECL_GLPROC(PFNGLCREATESHADERPROC,                                  glCreateShader,                                 GLuint,         (GLenum));
DECL_GLPROC(PFNGLSHADERSOURCEPROC,                                  glShaderSource,                                 void,           (GLuint, GLsizei, const GLchar* const*, const GLint*));
DECL_GLPROC(PFNGLCOMPILESHADERPROC,                                 glCompileShader,                                void,           (GLuint));
DECL_GLPROC(PFNGLGETSHADERIVPROC,                                   glGetShaderiv,                                  void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETSHADERINFOLOGPROC,                              glGetShaderInfoLog,                             void,           (GLuint, GLsizei, GLsizei*, GLchar*));
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

/* **GL_ARB_shader_objects** TODO: which extension from GL 3.0 ??? */

DECL_GLPROC(PFNGLUNIFORM1UIVPROC,                                   glUniform1uiv,                                  void,           (GLint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLUNIFORM2UIVPROC,                                   glUniform2uiv,                                  void,           (GLint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLUNIFORM3UIVPROC,                                   glUniform3uiv,                                  void,           (GLint, GLsizei, const GLuint*));
DECL_GLPROC(PFNGLUNIFORM4UIVPROC,                                   glUniform4uiv,                                  void,           (GLint, GLsizei, const GLuint*));

/* **GL_ARB_shader_objects** TODO: which extension from GL 4.0 ??? */

DECL_GLPROC(PFNGLUNIFORM1DVPROC,                                    glUniform1dv,                                   void,           (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORM2DVPROC,                                    glUniform2dv,                                   void,           (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORM3DVPROC,                                    glUniform3dv,                                   void,           (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORM4DVPROC,                                    glUniform4dv,                                   void,           (GLint, GLsizei, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX2DVPROC,                              glUniformMatrix2dv,                             void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3DVPROC,                              glUniformMatrix3dv,                             void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4DVPROC,                              glUniformMatrix4dv,                             void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX2X3DVPROC,                            glUniformMatrix2x3dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX2X4DVPROC,                            glUniformMatrix2x4dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3X2DVPROC,                            glUniformMatrix3x2dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX3X4DVPROC,                            glUniformMatrix3x4dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4X2DVPROC,                            glUniformMatrix4x2dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));
DECL_GLPROC(PFNGLUNIFORMMATRIX4X3DVPROC,                            glUniformMatrix4x3dv,                           void,           (GLint, GLsizei, GLboolean, const GLdouble*));

/* GL_ARB_tessellation_shader */

DECL_GLPROC(PFNGLPATCHPARAMETERIPROC,                               glPatchParameteri,                              void,           (GLenum, GLint));
DECL_GLPROC(PFNGLPATCHPARAMETERFVPROC,                              glPatchParameterfv,                             void,           (GLenum, const GLfloat*));

/* GL_ARB_compute_shader */

DECL_GLPROC(PFNGLDISPATCHCOMPUTEPROC,                               glDispatchCompute,                              void,           (GLuint, GLuint, GLuint));
DECL_GLPROC(PFNGLDISPATCHCOMPUTEINDIRECTPROC,                       glDispatchComputeIndirect,                      void,           (GLintptr));

/* GL_ARB_get_program_binary */

DECL_GLPROC(PFNGLGETPROGRAMBINARYPROC,                              glGetProgramBinary,                             void,           (GLuint, GLsizei, GLsizei*, GLenum*, void*));
DECL_GLPROC(PFNGLPROGRAMBINARYPROC,                                 glProgramBinary,                                void,           (GLuint, GLenum, const void*, GLsizei));
DECL_GLPROC(PFNGLPROGRAMPARAMETERIPROC,                             glProgramParameteri,                            void,           (GLuint, GLenum, GLint));

/* GL_ARB_program_interface_query */

DECL_GLPROC(PFNGLGETPROGRAMINTERFACEIVPROC,                         glGetProgramInterfaceiv,                        void,           (GLuint, GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCEINDEXPROC,                       glGetProgramResourceIndex,                      GLuint,         (GLuint, GLenum, const GLchar*));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCENAMEPROC,                        glGetProgramResourceName,                       void,           (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCEIVPROC,                          glGetProgramResourceiv,                         void,           (GLuint, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLint*));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCELOCATIONPROC,                    glGetProgramResourceLocation,                   GLint,          (GLuint, GLenum, const GLchar*));
DECL_GLPROC(PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC,               glGetProgramResourceLocationIndex,              GLint,          (GLuint, GLenum, const GLchar*));

/* GL_ARB_uniform_buffer_object */

DECL_GLPROC(PFNGLGETUNIFORMBLOCKINDEXPROC,                          glGetUniformBlockIndex,                         GLuint,         (GLuint, const GLchar*));
DECL_GLPROC(PFNGLGETACTIVEUNIFORMBLOCKIVPROC,                       glGetActiveUniformBlockiv,                      void,           (GLuint, GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC,                     glGetActiveUniformBlockName,                    void,           (GLuint, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLUNIFORMBLOCKBINDINGPROC,                           glUniformBlockBinding,                          void,           (GLuint, GLuint, GLuint));
DECL_GLPROC(PFNGLBINDBUFFERBASEPROC,                                glBindBufferBase,                               void,           (GLenum, GLuint, GLuint));

/* GL_ARB_shader_storage_buffer_object */

DECL_GLPROC(PFNGLSHADERSTORAGEBLOCKBINDINGPROC,                     glShaderStorageBlockBinding,                    void,           (GLuint, GLuint, GLuint));

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

/* GL_ARB_viewport_array */

DECL_GLPROC(PFNGLVIEWPORTARRAYVPROC,                                glViewportArrayv,                               void,           (GLuint, GLsizei, const GLfloat*));
DECL_GLPROC(PFNGLSCISSORARRAYVPROC,                                 glScissorArrayv,                                void,           (GLuint, GLsizei, const GLint*));
DECL_GLPROC(PFNGLDEPTHRANGEARRAYVPROC,                              glDepthRangeArrayv,                             void,           (GLuint, GLsizei, const GLdouble*));

/* GL_ATI_separate_stencil ??? */

DECL_GLPROC(PFNGLSTENCILFUNCSEPARATEPROC,                           glStencilFuncSeparate,                          void,           (GLenum, GLenum, GLint, GLuint));
DECL_GLPROC(PFNGLSTENCILMASKSEPARATEPROC,                           glStencilMaskSeparate,                          void,           (GLenum, GLuint));
DECL_GLPROC(PFNGLSTENCILOPSEPARATEPROC,                             glStencilOpSeparate,                            void,           (GLenum, GLenum, GLenum, GLenum));

/* GL_KHR_debug */

//NOTE: "KHR" suffix is required for OpenGL ES
DECL_GLPROC(PFNGLDEBUGMESSAGECONTROLPROC,                           glDebugMessageControl,                          void,           (GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean));
DECL_GLPROC(PFNGLDEBUGMESSAGEINSERTPROC,                            glDebugMessageInsert,                           void,           (GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*));
DECL_GLPROC(PFNGLDEBUGMESSAGECALLBACKPROC,                          glDebugMessageCallback,                         void,           (GLDEBUGPROC,                                                     const void*));
DECL_GLPROC(PFNGLGETDEBUGMESSAGELOGPROC,                            glGetDebugMessageLog,                           GLuint,         (GLuint, GLsizei, GLenum*, GLenum*, GLuint*, GLenum*, GLsizei*, GLchar*));
//DECL_GLPROC(PFNGLGETPOINTERVPROC,                                   glGetPointerv,                                  void,           (GLenum, GLvoid**)); // only glGetPointervEXT version???
DECL_GLPROC(PFNGLPUSHDEBUGGROUPPROC,                                glPushDebugGroup,                               void,           (GLenum, GLuint, GLsizei, const GLchar*));
DECL_GLPROC(PFNGLPOPDEBUGGROUPPROC,                                 glPopDebugGroup,                                void,           (void));
DECL_GLPROC(PFNGLOBJECTLABELPROC,                                   glObjectLabel,                                  void,           (GLenum, GLuint, GLsizei, const GLchar*));
DECL_GLPROC(PFNGLGETOBJECTLABELPROC,                                glGetObjectLabel,                               void,           (GLenum, GLuint, GLsizei, GLsizei*, GLchar*));
DECL_GLPROC(PFNGLOBJECTPTRLABELPROC,                                glObjectPtrLabel,                               void,           (const void*, GLsizei, const GLchar*));
DECL_GLPROC(PFNGLGETOBJECTPTRLABELPROC,                             glGetObjectPtrLabel,                            void,           (const void*, GLsizei, GLsizei*, GLchar*));

/* GL_ARB_clip_control */

DECL_GLPROC(PFNGLCLIPCONTROLPROC,                                   glClipControl,                                  void,           (GLenum, GLenum));

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

/* GL_ARB_gl_spirv */

DECL_GLPROC(PFNGLSPECIALIZESHADERPROC,                              glSpecializeShader,                             void,           (GLuint, const GLchar*, GLuint, const GLuint*, const GLuint*));

/* GL_ARB_texture_storage */

DECL_GLPROC(PFNGLTEXSTORAGE1DPROC,                                  glTexStorage1D,                                 void,           (GLenum, GLsizei, GLenum, GLsizei));
DECL_GLPROC(PFNGLTEXSTORAGE2DPROC,                                  glTexStorage2D,                                 void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLTEXSTORAGE3DPROC,                                  glTexStorage3D,                                 void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei));

/* GL_ARB_texture_storage_multisample */

DECL_GLPROC(PFNGLTEXSTORAGE2DMULTISAMPLEPROC,                       glTexStorage2DMultisample,                      void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(PFNGLTEXSTORAGE3DMULTISAMPLEPROC,                       glTexStorage3DMultisample,                      void,           (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));

/* GL_ARB_buffer_storage */

DECL_GLPROC(PFNGLBUFFERSTORAGEPROC,                                 glBufferStorage,                                void,           (GLenum, GLsizeiptr, const void*, GLbitfield));

/* GL_ARB_copy_buffer */

DECL_GLPROC(PFNGLCOPYBUFFERSUBDATAPROC,                             glCopyBufferSubData,                            void,           (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr));

/* GL_ARB_copy_image */

DECL_GLPROC(PFNGLCOPYIMAGESUBDATAPROC,                              glCopyImageSubData,                             void,           (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei));

/* GL_ARB_polygon_offset_clamp */

DECL_GLPROC(PFNGLPOLYGONOFFSETCLAMPPROC,                            glPolygonOffsetClamp,                           void,           (GLfloat, GLfloat, GLfloat));

/* GL_ARB_shader_image_load_store */

DECL_GLPROC(PFNGLBINDIMAGETEXTUREPROC,                              glBindImageTexture,                             void,           (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum));
DECL_GLPROC(PFNGLMEMORYBARRIERPROC,                                 glMemoryBarrier,                                void,           (GLbitfield));

/* GL_ARB_framebuffer_no_attachments */

DECL_GLPROC(PFNGLFRAMEBUFFERPARAMETERIPROC,                         glFramebufferParameteri,                        void,           (GLenum, GLenum, GLint));
DECL_GLPROC(PFNGLGETFRAMEBUFFERPARAMETERIVPROC,                     glGetFramebufferParameteriv,                    void,           (GLenum, GLenum, GLint*));

/* GL_ARB_clear_buffer_object */

DECL_GLPROC(PFNGLCLEARBUFFERDATAPROC,                               glClearBufferData,                              void,           (GLenum, GLenum, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLCLEARBUFFERSUBDATAPROC,                            glClearBufferSubData,                           void,           (GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*));

/* GL_ARB_draw_indirect */

DECL_GLPROC(PFNGLDRAWARRAYSINDIRECTPROC,                            glDrawArraysIndirect,                           void,           (GLenum, const void*));
DECL_GLPROC(PFNGLDRAWELEMENTSINDIRECTPROC,                          glDrawElementsIndirect,                         void,           (GLenum, GLenum, const void*));

/* GL_ARB_multi_draw_indirect */

DECL_GLPROC(PFNGLMULTIDRAWARRAYSINDIRECTPROC,                       glMultiDrawArraysIndirect,                      void,           (GLenum, const void*, GLsizei, GLsizei));
DECL_GLPROC(PFNGLMULTIDRAWELEMENTSINDIRECTPROC,                     glMultiDrawElementsIndirect,                    void,           (GLenum, GLenum, const void*, GLsizei, GLsizei));

/* GL_ARB_get_texture_sub_image */

DECL_GLPROC(PFNGLGETTEXTURESUBIMAGEPROC,                            glGetTextureSubImage,                           void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void*));
DECL_GLPROC(PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC,                  glGetCompressedTextureSubImage,                 void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, void*));

/* GL_ARB_direct_state_access */

DECL_GLPROC(PFNGLCREATETRANSFORMFEEDBACKSPROC,                      glCreateTransformFeedbacks,                     void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC,                   glTransformFeedbackBufferBase,                  void,           (GLuint, GLuint, GLuint));
DECL_GLPROC(PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC,                  glTransformFeedbackBufferRange,                 void,           (GLuint, GLuint, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLGETTRANSFORMFEEDBACKIVPROC,                        glGetTransformFeedbackiv,                       void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETTRANSFORMFEEDBACKI_VPROC,                       glGetTransformFeedbacki_v,                      void,           (GLuint, GLenum, GLuint, GLint*));
DECL_GLPROC(PFNGLGETTRANSFORMFEEDBACKI64_VPROC,                     glGetTransformFeedbacki64_v,                    void,           (GLuint, GLenum, GLuint, GLint64*));
DECL_GLPROC(PFNGLCREATEBUFFERSPROC,                                 glCreateBuffers,                                void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLNAMEDBUFFERSTORAGEPROC,                            glNamedBufferStorage,                           void,           (GLuint, GLsizeiptr, const void*, GLbitfield));
DECL_GLPROC(PFNGLNAMEDBUFFERDATAPROC,                               glNamedBufferData,                              void,           (GLuint, GLsizeiptr, const void*, GLenum));
DECL_GLPROC(PFNGLNAMEDBUFFERSUBDATAPROC,                            glNamedBufferSubData,                           void,           (GLuint, GLintptr, GLsizeiptr, const void*));
DECL_GLPROC(PFNGLCOPYNAMEDBUFFERSUBDATAPROC,                        glCopyNamedBufferSubData,                       void,           (GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLCLEARNAMEDBUFFERDATAPROC,                          glClearNamedBufferData,                         void,           (GLuint, GLenum, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLCLEARNAMEDBUFFERSUBDATAPROC,                       glClearNamedBufferSubData,                      void,           (GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLMAPNAMEDBUFFERPROC,                                glMapNamedBuffer,                               void*,          (GLuint, GLenum));
DECL_GLPROC(PFNGLMAPNAMEDBUFFERRANGEPROC,                           glMapNamedBufferRange,                          void*,          (GLuint, GLintptr, GLsizeiptr, GLbitfield));
DECL_GLPROC(PFNGLUNMAPNAMEDBUFFERPROC,                              glUnmapNamedBuffer,                             GLboolean,      (GLuint));
DECL_GLPROC(PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC,                   glFlushMappedNamedBufferRange,                  void,           (GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLGETNAMEDBUFFERPARAMETERIVPROC,                     glGetNamedBufferParameteriv,                    void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETNAMEDBUFFERPARAMETERI64VPROC,                   glGetNamedBufferParameteri64v,                  void,           (GLuint, GLenum, GLint64*));
DECL_GLPROC(PFNGLGETNAMEDBUFFERPOINTERVPROC,                        glGetNamedBufferPointerv,                       void,           (GLuint, GLenum, void**));
DECL_GLPROC(PFNGLGETNAMEDBUFFERSUBDATAPROC,                         glGetNamedBufferSubData,                        void,           (GLuint, GLintptr, GLsizeiptr, void*));
DECL_GLPROC(PFNGLCREATEFRAMEBUFFERSPROC,                            glCreateFramebuffers,                           void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC,                  glNamedFramebufferRenderbuffer,                 void,           (GLuint, GLenum, GLenum, GLuint));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC,                    glNamedFramebufferParameteri,                   void,           (GLuint, GLenum, GLint));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERTEXTUREPROC,                       glNamedFramebufferTexture,                      void,           (GLuint, GLenum, GLuint, GLint));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC,                  glNamedFramebufferTextureLayer,                 void,           (GLuint, GLenum, GLuint, GLint, GLint));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC,                    glNamedFramebufferDrawBuffer,                   void,           (GLuint, GLenum));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC,                   glNamedFramebufferDrawBuffers,                  void,           (GLuint, GLsizei, const GLenum*));
DECL_GLPROC(PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC,                    glNamedFramebufferReadBuffer,                   void,           (GLuint, GLenum));
DECL_GLPROC(PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC,                glInvalidateNamedFramebufferData,               void,           (GLuint, GLsizei, const GLenum*));
DECL_GLPROC(PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC,             glInvalidateNamedFramebufferSubData,            void,           (GLuint, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(PFNGLCLEARNAMEDFRAMEBUFFERIVPROC,                       glClearNamedFramebufferiv,                      void,           (GLuint, GLenum, GLint, const GLint*));
DECL_GLPROC(PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC,                      glClearNamedFramebufferuiv,                     void,           (GLuint, GLenum, GLint, const GLuint*));
DECL_GLPROC(PFNGLCLEARNAMEDFRAMEBUFFERFVPROC,                       glClearNamedFramebufferfv,                      void,           (GLuint, GLenum, GLint, const GLfloat*));
DECL_GLPROC(PFNGLCLEARNAMEDFRAMEBUFFERFIPROC,                       glClearNamedFramebufferfi,                      void,           (GLuint, GLenum, GLint, GLfloat, GLint));
DECL_GLPROC(PFNGLBLITNAMEDFRAMEBUFFERPROC,                          glBlitNamedFramebuffer,                         void,           (GLuint, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum));
DECL_GLPROC(PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC,                   glCheckNamedFramebufferStatus,                  GLenum,         (GLuint, GLenum));
DECL_GLPROC(PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC,                glGetNamedFramebufferParameteriv,               void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC,      glGetNamedFramebufferAttachmentParameteriv,     void,           (GLuint, GLenum, GLenum, GLint*));
DECL_GLPROC(PFNGLCREATERENDERBUFFERSPROC,                           glCreateRenderbuffers,                          void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLNAMEDRENDERBUFFERSTORAGEPROC,                      glNamedRenderbufferStorage,                     void,           (GLuint, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC,           glNamedRenderbufferStorageMultisample,          void,           (GLuint, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC,               glGetNamedRenderbufferParameteriv,              void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLCREATETEXTURESPROC,                                glCreateTextures,                               void,           (GLenum, GLsizei, GLuint*));
DECL_GLPROC(PFNGLTEXTUREBUFFERPROC,                                 glTextureBuffer,                                void,           (GLuint, GLenum, GLuint));
DECL_GLPROC(PFNGLTEXTUREBUFFERRANGEPROC,                            glTextureBufferRange,                           void,           (GLuint, GLenum, GLuint, GLintptr, GLsizeiptr));
DECL_GLPROC(PFNGLTEXTURESTORAGE1DPROC,                              glTextureStorage1D,                             void,           (GLuint, GLsizei, GLenum, GLsizei));
DECL_GLPROC(PFNGLTEXTURESTORAGE2DPROC,                              glTextureStorage2D,                             void,           (GLuint, GLsizei, GLenum, GLsizei, GLsizei));
DECL_GLPROC(PFNGLTEXTURESTORAGE3DPROC,                              glTextureStorage3D,                             void,           (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei));
DECL_GLPROC(PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC,                   glTextureStorage2DMultisample,                  void,           (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC,                   glTextureStorage3DMultisample,                  void,           (GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean));
DECL_GLPROC(PFNGLTEXTURESUBIMAGE1DPROC,                             glTextureSubImage1D,                            void,           (GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLTEXTURESUBIMAGE2DPROC,                             glTextureSubImage2D,                            void,           (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLTEXTURESUBIMAGE3DPROC,                             glTextureSubImage3D,                            void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC,                   glCompressedTextureSubImage1D,                  void,           (GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC,                   glCompressedTextureSubImage2D,                  void,           (GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC,                   glCompressedTextureSubImage3D,                  void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void*));
DECL_GLPROC(PFNGLCOPYTEXTURESUBIMAGE1DPROC,                         glCopyTextureSubImage1D,                        void,           (GLuint, GLint, GLint, GLint, GLint, GLsizei));
DECL_GLPROC(PFNGLCOPYTEXTURESUBIMAGE2DPROC,                         glCopyTextureSubImage2D,                        void,           (GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(PFNGLCOPYTEXTURESUBIMAGE3DPROC,                         glCopyTextureSubImage3D,                        void,           (GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei));
DECL_GLPROC(PFNGLTEXTUREPARAMETERFPROC,                             glTextureParameterf,                            void,           (GLuint, GLenum, GLfloat));
DECL_GLPROC(PFNGLTEXTUREPARAMETERFVPROC,                            glTextureParameterfv,                           void,           (GLuint, GLenum, const GLfloat*));
DECL_GLPROC(PFNGLTEXTUREPARAMETERIPROC,                             glTextureParameteri,                            void,           (GLuint, GLenum, GLint));
DECL_GLPROC(PFNGLTEXTUREPARAMETERIIVPROC,                           glTextureParameterIiv,                          void,           (GLuint, GLenum, const GLint*));
DECL_GLPROC(PFNGLTEXTUREPARAMETERIUIVPROC,                          glTextureParameterIuiv,                         void,           (GLuint, GLenum, const GLuint*));
DECL_GLPROC(PFNGLTEXTUREPARAMETERIVPROC,                            glTextureParameteriv,                           void,           (GLuint, GLenum, const GLint*));
DECL_GLPROC(PFNGLGENERATETEXTUREMIPMAPPROC,                         glGenerateTextureMipmap,                        void,           (GLuint));
DECL_GLPROC(PFNGLBINDTEXTUREUNITPROC,                               glBindTextureUnit,                              void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLGETTEXTUREIMAGEPROC,                               glGetTextureImage,                              void,           (GLuint, GLint, GLenum, GLenum, GLsizei, void*));
DECL_GLPROC(PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC,                     glGetCompressedTextureImage,                    void,           (GLuint, GLint, GLsizei, void*));
DECL_GLPROC(PFNGLGETTEXTURELEVELPARAMETERFVPROC,                    glGetTextureLevelParameterfv,                   void,           (GLuint, GLint, GLenum, GLfloat*));
DECL_GLPROC(PFNGLGETTEXTURELEVELPARAMETERIVPROC,                    glGetTextureLevelParameteriv,                   void,           (GLuint, GLint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETTEXTUREPARAMETERFVPROC,                         glGetTextureParameterfv,                        void,           (GLuint, GLenum, GLfloat*));
DECL_GLPROC(PFNGLGETTEXTUREPARAMETERIIVPROC,                        glGetTextureParameterIiv,                       void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETTEXTUREPARAMETERIUIVPROC,                       glGetTextureParameterIuiv,                      void,           (GLuint, GLenum, GLuint*));
DECL_GLPROC(PFNGLGETTEXTUREPARAMETERIVPROC,                         glGetTextureParameteriv,                        void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLCREATEVERTEXARRAYSPROC,                            glCreateVertexArrays,                           void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLDISABLEVERTEXARRAYATTRIBPROC,                      glDisableVertexArrayAttrib,                     void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLENABLEVERTEXARRAYATTRIBPROC,                       glEnableVertexArrayAttrib,                      void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYELEMENTBUFFERPROC,                      glVertexArrayElementBuffer,                     void,           (GLuint, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYVERTEXBUFFERPROC,                       glVertexArrayVertexBuffer,                      void,           (GLuint, GLuint, GLuint, GLintptr, GLsizei));
DECL_GLPROC(PFNGLVERTEXARRAYVERTEXBUFFERSPROC,                      glVertexArrayVertexBuffers,                     void,           (GLuint, GLuint, GLsizei, const GLuint*, const GLintptr*, const GLsizei*));
DECL_GLPROC(PFNGLVERTEXARRAYATTRIBFORMATPROC,                       glVertexArrayAttribFormat,                      void,           (GLuint, GLuint, GLint, GLenum, GLboolean, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYATTRIBIFORMATPROC,                      glVertexArrayAttribIFormat,                     void,           (GLuint, GLuint, GLint, GLenum, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYATTRIBLFORMATPROC,                      glVertexArrayAttribLFormat,                     void,           (GLuint, GLuint, GLint, GLenum, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYATTRIBBINDINGPROC,                      glVertexArrayAttribBinding,                     void,           (GLuint, GLuint, GLuint));
DECL_GLPROC(PFNGLVERTEXARRAYBINDINGDIVISORPROC,                     glVertexArrayBindingDivisor,                    void,           (GLuint, GLuint, GLuint));
DECL_GLPROC(PFNGLGETVERTEXARRAYIVPROC,                              glGetVertexArrayiv,                             void,           (GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETVERTEXARRAYINDEXEDIVPROC,                       glGetVertexArrayIndexediv,                      void,           (GLuint, GLuint, GLenum, GLint*));
DECL_GLPROC(PFNGLGETVERTEXARRAYINDEXED64IVPROC,                     glGetVertexArrayIndexed64iv,                    void,           (GLuint, GLuint, GLenum, GLint64*));
DECL_GLPROC(PFNGLCREATESAMPLERSPROC,                                glCreateSamplers,                               void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLCREATEPROGRAMPIPELINESPROC,                        glCreateProgramPipelines,                       void,           (GLsizei, GLuint*));
DECL_GLPROC(PFNGLCREATEQUERIESPROC,                                 glCreateQueries,                                void,           (GLenum, GLsizei, GLuint*));
DECL_GLPROC(PFNGLGETQUERYBUFFEROBJECTIVPROC,                        glGetQueryBufferObjectiv,                       void,           (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(PFNGLGETQUERYBUFFEROBJECTUIVPROC,                       glGetQueryBufferObjectuiv,                      void,           (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(PFNGLGETQUERYBUFFEROBJECTI64VPROC,                      glGetQueryBufferObjecti64v,                     void,           (GLuint, GLuint, GLenum, GLintptr));
DECL_GLPROC(PFNGLGETQUERYBUFFEROBJECTUI64VPROC,                     glGetQueryBufferObjectui64v,                    void,           (GLuint, GLuint, GLenum, GLintptr));

#undef DECL_GLPROC

#endif // /__APPLE__



// ================================================================================
