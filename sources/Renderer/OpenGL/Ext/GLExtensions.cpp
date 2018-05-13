/*
 * GLExtensions.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensions.h"


namespace LLGL
{

    
#ifndef LLGL_OS_MACOS

/* Platform specific GL extensions */

#if defined(LLGL_OS_WIN32)

// WGL_EXT_swap_control
PFNWGLSWAPINTERVALEXTPROC                               wglSwapIntervalEXT                              = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC                          wglChoosePixelFormatARB                         = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC                       wglCreateContextAttribsARB                      = nullptr;
PFNWGLGETEXTENSIONSSTRINGARBPROC                        wglGetExtensionsStringARB                       = nullptr;

#elif defined(LLGL_OS_LINUX)

// GLX_SGI_swap_control
PFNGLXSWAPINTERVALSGIPROC                               glXSwapIntervalSGI                              = nullptr;

#endif

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

/* GL 3.0 extensions (for Core Profile) */

PFNGLGETSTRINGIPROC                                     glGetStringi                                    = nullptr;

#endif

/* GL_EXT_blend_func_separate */

PFNGLBLENDFUNCSEPARATEPROC                              glBlendFuncSeparate                             = nullptr;

/* GL_EXT_blend_minmax */

PFNGLBLENDEQUATIONPROC                                  glBlendEquation                                 = nullptr;

/* GL_EXT_blend_color */

PFNGLBLENDCOLORPROC                                     glBlendColor                                    = nullptr;

/* GL_EXT_blend_equation_separate */

PFNGLBLENDEQUATIONSEPARATEPROC                          glBlendEquationSeparate                         = nullptr;

/* GL_ARB_draw_buffers */

PFNGLDRAWBUFFERSPROC                                    glDrawBuffers                                   = nullptr;

/* GL_EXT_draw_buffers2 */

PFNGLCOLORMASKIPROC                                     glColorMaski                                    = nullptr;
PFNGLGETBOOLEANI_VPROC                                  glGetBooleani_v                                 = nullptr;
PFNGLGETINTEGERI_VPROC                                  glGetIntegeri_v                                 = nullptr;
PFNGLENABLEIPROC                                        glEnablei                                       = nullptr;
PFNGLDISABLEIPROC                                       glDisablei                                      = nullptr;
PFNGLISENABLEDIPROC                                     glIsEnabledi                                    = nullptr;

/* GL_ARB_draw_buffers_blend */

PFNGLBLENDEQUATIONIPROC                                 glBlendEquationi                                = nullptr;
PFNGLBLENDEQUATIONSEPARATEIPROC                         glBlendEquationSeparatei                        = nullptr;
PFNGLBLENDFUNCIPROC                                     glBlendFunci                                    = nullptr;
PFNGLBLENDFUNCSEPARATEIPROC                             glBlendFuncSeparatei                            = nullptr;

/* GL_ARB_multitexture */

PFNGLACTIVETEXTUREPROC                                  glActiveTexture                                 = nullptr;

/* GL_EXT_texture3D */

PFNGLTEXIMAGE3DPROC                                     glTexImage3D                                    = nullptr;
PFNGLTEXSUBIMAGE3DPROC                                  glTexSubImage3D                                 = nullptr;

/* GL_ARB_clear_texture */

PFNGLCLEARTEXIMAGEPROC                                  glClearTexImage                                 = nullptr;
PFNGLCLEARTEXSUBIMAGEPROC                               glClearTexSubImage                              = nullptr;

/* GL_ARB_texture_compression */

PFNGLCOMPRESSEDTEXIMAGE1DPROC                           glCompressedTexImage1D                          = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC                           glCompressedTexImage2D                          = nullptr;
PFNGLCOMPRESSEDTEXIMAGE3DPROC                           glCompressedTexImage3D                          = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                        glCompressedTexSubImage1D                       = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                        glCompressedTexSubImage2D                       = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                        glCompressedTexSubImage3D                       = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC                          glGetCompressedTexImage                         = nullptr;

/* GL_ARB_texture_multisample */

PFNGLTEXIMAGE2DMULTISAMPLEPROC                          glTexImage2DMultisample                         = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLEPROC                          glTexImage3DMultisample                         = nullptr;
PFNGLGETMULTISAMPLEFVPROC                               glGetMultisamplefv                              = nullptr;
PFNGLSAMPLEMASKIPROC                                    glSampleMaski                                   = nullptr;

/* GL_ARB_sampler_objects */

PFNGLGENSAMPLERSPROC                                    glGenSamplers                                   = nullptr;
PFNGLDELETESAMPLERSPROC                                 glDeleteSamplers                                = nullptr;
PFNGLBINDSAMPLERPROC                                    glBindSampler                                   = nullptr;
PFNGLSAMPLERPARAMETERIPROC                              glSamplerParameteri                             = nullptr;
PFNGLSAMPLERPARAMETERFPROC                              glSamplerParameterf                             = nullptr;
PFNGLSAMPLERPARAMETERIVPROC                             glSamplerParameteriv                            = nullptr;
PFNGLSAMPLERPARAMETERFVPROC                             glSamplerParameterfv                            = nullptr;

/* GL_ARB_multi_bind */

PFNGLBINDBUFFERSBASEPROC                                glBindBuffersBase                               = nullptr;
PFNGLBINDBUFFERSRANGEPROC                               glBindBuffersRange                              = nullptr;
PFNGLBINDTEXTURESPROC                                   glBindTextures                                  = nullptr;
PFNGLBINDSAMPLERSPROC                                   glBindSamplers                                  = nullptr;
PFNGLBINDIMAGETEXTURESPROC                              glBindImageTextures                             = nullptr;
PFNGLBINDVERTEXBUFFERSPROC                              glBindVertexBuffers                             = nullptr;

/* GL_ARB_vertex_buffer_object */

PFNGLGENBUFFERSPROC                                     glGenBuffers                                    = nullptr;
PFNGLDELETEBUFFERSPROC                                  glDeleteBuffers                                 = nullptr;
PFNGLBINDBUFFERPROC                                     glBindBuffer                                    = nullptr;
PFNGLBUFFERDATAPROC                                     glBufferData                                    = nullptr;
PFNGLBUFFERSUBDATAPROC                                  glBufferSubData                                 = nullptr;
PFNGLMAPBUFFERPROC                                      glMapBuffer                                     = nullptr;
PFNGLUNMAPBUFFERPROC                                    glUnmapBuffer                                   = nullptr;

/* GL_ARB_vertex_buffer_object ??? */

PFNGLENABLEVERTEXATTRIBARRAYPROC                        glEnableVertexAttribArray                       = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC                       glDisableVertexAttribArray                      = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC                            glVertexAttribPointer                           = nullptr;
PFNGLBINDATTRIBLOCATIONPROC                             glBindAttribLocation                            = nullptr;

/* GL_EXT_gpu_shader4 */

PFNGLVERTEXATTRIBIPOINTERPROC                           glVertexAttribIPointer                          = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC                           glBindFragDataLocation                          = nullptr;
PFNGLGETFRAGDATALOCATIONPROC                            glGetFragDataLocation                           = nullptr;

/* GL_ARB_instanced_arrays */

PFNGLVERTEXATTRIBDIVISORPROC                            glVertexAttribDivisor                           = nullptr;

/* GL_ARB_vertex_array_object */

PFNGLGENVERTEXARRAYSPROC                                glGenVertexArrays                               = nullptr;
PFNGLDELETEVERTEXARRAYSPROC                             glDeleteVertexArrays                            = nullptr;
PFNGLBINDVERTEXARRAYPROC                                glBindVertexArray                               = nullptr;

/* GL_ARB_framebuffer_object */

PFNGLGENRENDERBUFFERSPROC                               glGenRenderbuffers                              = nullptr;
PFNGLDELETERENDERBUFFERSPROC                            glDeleteRenderbuffers                           = nullptr;
PFNGLBINDRENDERBUFFERPROC                               glBindRenderbuffer                              = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC                            glRenderbufferStorage                           = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC                 glRenderbufferStorageMultisample                = nullptr;
PFNGLGENFRAMEBUFFERSPROC                                glGenFramebuffers                               = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC                             glDeleteFramebuffers                            = nullptr;
PFNGLBINDFRAMEBUFFERPROC                                glBindFramebuffer                               = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC                         glCheckFramebufferStatus                        = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC                             glFramebufferTexture                            = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC                           glFramebufferTexture1D                          = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC                           glFramebufferTexture2D                          = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC                           glFramebufferTexture3D                          = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC                        glFramebufferTextureLayer                       = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC                        glFramebufferRenderbuffer                       = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC            glGetFramebufferAttachmentParameteriv           = nullptr;
PFNGLBLITFRAMEBUFFERPROC                                glBlitFramebuffer                               = nullptr;
PFNGLGENERATEMIPMAPPROC                                 glGenerateMipmap                                = nullptr;

#if 1 //WHICH EXTENSION???
PFNGLCLEARBUFFERIVPROC                                  glClearBufferiv                                 = nullptr;
PFNGLCLEARBUFFERUIVPROC                                 glClearBufferuiv                                = nullptr;
PFNGLCLEARBUFFERFVPROC                                  glClearBufferfv                                 = nullptr;
PFNGLCLEARBUFFERFIPROC                                  glClearBufferfi                                 = nullptr;
#endif

/* GL_ARB_draw_instanced */

PFNGLDRAWARRAYSINSTANCEDPROC                            glDrawArraysInstanced                           = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC                          glDrawElementsInstanced                         = nullptr;

/* GL_ARB_draw_elements_base_vertex */

PFNGLDRAWELEMENTSBASEVERTEXPROC                         glDrawElementsBaseVertex                        = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC                glDrawElementsInstancedBaseVertex               = nullptr;

/* GL_ARB_base_instance */

PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC                glDrawArraysInstancedBaseInstance               = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC              glDrawElementsInstancedBaseInstance             = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC    glDrawElementsInstancedBaseVertexBaseInstance   = nullptr;

/* GL_ARB_shader_objects */

PFNGLCREATESHADERPROC                                   glCreateShader                                  = nullptr;
PFNGLSHADERSOURCEPROC                                   glShaderSource                                  = nullptr;
PFNGLCOMPILESHADERPROC                                  glCompileShader                                 = nullptr;
PFNGLGETSHADERIVPROC                                    glGetShaderiv                                   = nullptr;
PFNGLGETSHADERINFOLOGPROC                               glGetShaderInfoLog                              = nullptr;
PFNGLDELETESHADERPROC                                   glDeleteShader                                  = nullptr;
PFNGLCREATEPROGRAMPROC                                  glCreateProgram                                 = nullptr;
PFNGLDELETEPROGRAMPROC                                  glDeleteProgram                                 = nullptr;
PFNGLATTACHSHADERPROC                                   glAttachShader                                  = nullptr;
PFNGLDETACHSHADERPROC                                   glDetachShader                                  = nullptr;
PFNGLLINKPROGRAMPROC                                    glLinkProgram                                   = nullptr;
PFNGLVALIDATEPROGRAMPROC                                glValidateProgram                               = nullptr;
PFNGLGETPROGRAMIVPROC                                   glGetProgramiv                                  = nullptr;
PFNGLGETPROGRAMINFOLOGPROC                              glGetProgramInfoLog                             = nullptr;
PFNGLUSEPROGRAMPROC                                     glUseProgram                                    = nullptr;
PFNGLGETACTIVEATTRIBPROC                                glGetActiveAttrib                               = nullptr;
PFNGLGETATTRIBLOCATIONPROC                              glGetAttribLocation                             = nullptr;
PFNGLGETACTIVEUNIFORMARBPROC                            glGetActiveUniform                              = nullptr;
PFNGLGETUNIFORMLOCATIONPROC                             glGetUniformLocation                            = nullptr;
PFNGLGETATTACHEDSHADERSPROC                             glGetAttachedShaders                            = nullptr;
PFNGLUNIFORM1FVPROC                                     glUniform1fv                                    = nullptr;
PFNGLUNIFORM2FVPROC                                     glUniform2fv                                    = nullptr;
PFNGLUNIFORM3FVPROC                                     glUniform3fv                                    = nullptr;
PFNGLUNIFORM4FVPROC                                     glUniform4fv                                    = nullptr;
PFNGLUNIFORM1IVPROC                                     glUniform1iv                                    = nullptr;
PFNGLUNIFORM2IVPROC                                     glUniform2iv                                    = nullptr;
PFNGLUNIFORM3IVPROC                                     glUniform3iv                                    = nullptr;
PFNGLUNIFORM4IVPROC                                     glUniform4iv                                    = nullptr;
PFNGLUNIFORMMATRIX2FVPROC                               glUniformMatrix2fv                              = nullptr;
PFNGLUNIFORMMATRIX3FVPROC                               glUniformMatrix3fv                              = nullptr;
PFNGLUNIFORMMATRIX4FVPROC                               glUniformMatrix4fv                              = nullptr;

/* GL_ARB_tessellation_shader */

PFNGLPATCHPARAMETERIPROC                                glPatchParameteri                               = nullptr;
PFNGLPATCHPARAMETERFVPROC                               glPatchParameterfv                              = nullptr;

/* GL_ARB_compute_shader */

PFNGLDISPATCHCOMPUTEPROC                                glDispatchCompute                               = nullptr;
PFNGLDISPATCHCOMPUTEINDIRECTPROC                        glDispatchComputeIndirect                       = nullptr;

/* GL_ARB_get_program_binary */

PFNGLGETPROGRAMBINARYPROC                               glGetProgramBinary                              = nullptr;
PFNGLPROGRAMBINARYPROC                                  glProgramBinary                                 = nullptr;
PFNGLPROGRAMPARAMETERIPROC                              glProgramParameteri                             = nullptr;

/* GL_ARB_program_interface_query */

PFNGLGETPROGRAMINTERFACEIVPROC                          glGetProgramInterfaceiv                         = nullptr;
PFNGLGETPROGRAMRESOURCEINDEXPROC                        glGetProgramResourceIndex                       = nullptr;
PFNGLGETPROGRAMRESOURCENAMEPROC                         glGetProgramResourceName                        = nullptr;
PFNGLGETPROGRAMRESOURCEIVPROC                           glGetProgramResourceiv                          = nullptr;
PFNGLGETPROGRAMRESOURCELOCATIONPROC                     glGetProgramResourceLocation                    = nullptr;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC                glGetProgramResourceLocationIndex               = nullptr;

/* GL_ARB_uniform_buffer_object */

PFNGLGETUNIFORMBLOCKINDEXPROC                           glGetUniformBlockIndex                          = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC                        glGetActiveUniformBlockiv                       = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                      glGetActiveUniformBlockName                     = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC                            glUniformBlockBinding                           = nullptr;
PFNGLBINDBUFFERBASEPROC                                 glBindBufferBase                                = nullptr;

/* GL_ARB_shader_storage_buffer_object */

PFNGLSHADERSTORAGEBLOCKBINDINGPROC                      glShaderStorageBlockBinding                     = nullptr;

/* GL_ARB_occlusion_query */

PFNGLGENQUERIESPROC                                     glGenQueries                                    = nullptr;
PFNGLDELETEQUERIESPROC                                  glDeleteQueries                                 = nullptr;
PFNGLBEGINQUERYPROC                                     glBeginQuery                                    = nullptr;
PFNGLENDQUERYPROC                                       glEndQuery                                      = nullptr;
PFNGLGETQUERYOBJECTIVPROC                               glGetQueryObjectiv                              = nullptr;
PFNGLGETQUERYOBJECTUIVPROC                              glGetQueryObjectuiv                             = nullptr;

/* GL_NV_conditional_render */

PFNGLBEGINCONDITIONALRENDERPROC                         glBeginConditionalRender                        = nullptr;
PFNGLENDCONDITIONALRENDERPROC                           glEndConditionalRender                          = nullptr;

/* GL_ARB_timer_query */

PFNGLQUERYCOUNTERPROC                                   glQueryCounter                                  = nullptr;
PFNGLGETQUERYOBJECTI64VPROC                             glGetQueryObjecti64v                            = nullptr;
PFNGLGETQUERYOBJECTUI64VPROC                            glGetQueryObjectui64v                           = nullptr;

/* GL_ARB_viewport_array */

PFNGLVIEWPORTARRAYVPROC                                 glViewportArrayv                                = nullptr;
PFNGLSCISSORARRAYVPROC                                  glScissorArrayv                                 = nullptr;
PFNGLDEPTHRANGEARRAYVPROC                               glDepthRangeArrayv                              = nullptr;

/* GL_ATI_separate_stencil ??? */

PFNGLSTENCILFUNCSEPARATEPROC                            glStencilFuncSeparate                           = nullptr;
PFNGLSTENCILMASKSEPARATEPROC                            glStencilMaskSeparate                           = nullptr;
PFNGLSTENCILOPSEPARATEPROC                              glStencilOpSeparate                             = nullptr;

/* GL_KHR_debug */

PFNGLDEBUGMESSAGECALLBACKPROC                           glDebugMessageCallback                          = nullptr;

/* GL_ARB_clip_control */

PFNGLCLIPCONTROLPROC                                    glClipControl                                   = nullptr;

/* GL_EXT_transform_feedback */

PFNGLBINDBUFFERRANGEPROC                                glBindBufferRange                               = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKPROC                         glBeginTransformFeedback                        = nullptr;
PFNGLENDTRANSFORMFEEDBACKPROC                           glEndTransformFeedback                          = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC                      glTransformFeedbackVaryings                     = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC                    glGetTransformFeedbackVarying                   = nullptr;

/* GL_NV_transform_feedback */

PFNGLBINDBUFFERRANGENVPROC                              glBindBufferRangeNV                             = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKNVPROC                       glBeginTransformFeedbackNV                      = nullptr;
PFNGLENDTRANSFORMFEEDBACKNVPROC                         glEndTransformFeedbackNV                        = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC                    glTransformFeedbackVaryingsNV                   = nullptr;
PFNGLGETVARYINGLOCATIONNVPROC                           glGetVaryingLocationNV                          = nullptr;
PFNGLGETACTIVEVARYINGNVPROC                             glGetActiveVaryingNV                            = nullptr;

/* GL_ARB_sync */

PFNGLFENCESYNCPROC                                      glFenceSync                                     = nullptr;
PFNGLISSYNCPROC                                         glIsSync                                        = nullptr;
PFNGLDELETESYNCPROC                                     glDeleteSync                                    = nullptr;
PFNGLCLIENTWAITSYNCPROC                                 glClientWaitSync                                = nullptr;
PFNGLWAITSYNCPROC                                       glWaitSync                                      = nullptr;
PFNGLGETINTEGER64VPROC                                  glGetInteger64v                                 = nullptr;
PFNGLGETSYNCIVPROC                                      glGetSynciv                                     = nullptr;

/* GL_ARB_internalformat_query */

PFNGLGETINTERNALFORMATIVPROC                            glGetInternalformativ                           = nullptr;

/* GL_ARB_internalformat_query2 */

PFNGLGETINTERNALFORMATI64VPROC                          glGetInternalformati64v                         = nullptr;

/* GL_ARB_ES2_compatibility */

PFNGLRELEASESHADERCOMPILERPROC                          glReleaseShaderCompiler                         = nullptr;
PFNGLSHADERBINARYPROC                                   glShaderBinary                                  = nullptr;
PFNGLGETSHADERPRECISIONFORMATPROC                       glGetShaderPrecisionFormat                      = nullptr;
PFNGLDEPTHRANGEFPROC                                    glDepthRangef                                   = nullptr;
PFNGLCLEARDEPTHFPROC                                    glClearDepthf                                   = nullptr;

/* GL_ARB_gl_spirv */

PFNGLSPECIALIZESHADERPROC                               glSpecializeShader                              = nullptr;

/* GL_ARB_texture_storage */

PFNGLTEXSTORAGE1DPROC                                   glTexStorage1D                                  = nullptr;
PFNGLTEXSTORAGE2DPROC                                   glTexStorage2D                                  = nullptr;
PFNGLTEXSTORAGE3DPROC                                   glTexStorage3D                                  = nullptr;

/* GL_ARB_texture_storage_multisample */

PFNGLTEXSTORAGE2DMULTISAMPLEPROC                        glTexStorage2DMultisample                       = nullptr;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC                        glTexStorage3DMultisample                       = nullptr;

/* GL_ARB_buffer_storage */

PFNGLBUFFERSTORAGEPROC                                  glBufferStorage                                 = nullptr;

/* GL_ARB_direct_state_access */

PFNGLCREATETRANSFORMFEEDBACKSPROC                       glCreateTransformFeedbacks                      = nullptr;
PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC                    glTransformFeedbackBufferBase                   = nullptr;
PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC                   glTransformFeedbackBufferRange                  = nullptr;
PFNGLGETTRANSFORMFEEDBACKIVPROC                         glGetTransformFeedbackiv                        = nullptr;
PFNGLGETTRANSFORMFEEDBACKI_VPROC                        glGetTransformFeedbacki_v                       = nullptr;
PFNGLGETTRANSFORMFEEDBACKI64_VPROC                      glGetTransformFeedbacki64_v                     = nullptr;
PFNGLCREATEBUFFERSPROC                                  glCreateBuffers                                 = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC                             glNamedBufferStorage                            = nullptr;
PFNGLNAMEDBUFFERDATAPROC                                glNamedBufferData                               = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC                             glNamedBufferSubData                            = nullptr;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC                         glCopyNamedBufferSubData                        = nullptr;
PFNGLCLEARNAMEDBUFFERDATAPROC                           glClearNamedBufferData                          = nullptr;
PFNGLCLEARNAMEDBUFFERSUBDATAPROC                        glClearNamedBufferSubData                       = nullptr;
PFNGLMAPNAMEDBUFFERPROC                                 glMapNamedBuffer                                = nullptr;
PFNGLMAPNAMEDBUFFERRANGEPROC                            glMapNamedBufferRange                           = nullptr;
PFNGLUNMAPNAMEDBUFFERPROC                               glUnmapNamedBuffer                              = nullptr;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC                    glFlushMappedNamedBufferRange                   = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC                      glGetNamedBufferParameteriv                     = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC                    glGetNamedBufferParameteri64v                   = nullptr;
PFNGLGETNAMEDBUFFERPOINTERVPROC                         glGetNamedBufferPointerv                        = nullptr;
PFNGLGETNAMEDBUFFERSUBDATAPROC                          glGetNamedBufferSubData                         = nullptr;
PFNGLCREATEFRAMEBUFFERSPROC                             glCreateFramebuffers                            = nullptr;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC                   glNamedFramebufferRenderbuffer                  = nullptr;
PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC                     glNamedFramebufferParameteri                    = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                        glNamedFramebufferTexture                       = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC                   glNamedFramebufferTextureLayer                  = nullptr;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC                     glNamedFramebufferDrawBuffer                    = nullptr;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC                    glNamedFramebufferDrawBuffers                   = nullptr;
PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC                     glNamedFramebufferReadBuffer                    = nullptr;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC                 glInvalidateNamedFramebufferData                = nullptr;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC              glInvalidateNamedFramebufferSubData             = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                        glClearNamedFramebufferiv                       = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                       glClearNamedFramebufferuiv                      = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                        glClearNamedFramebufferfv                       = nullptr;
PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                        glClearNamedFramebufferfi                       = nullptr;
PFNGLBLITNAMEDFRAMEBUFFERPROC                           glBlitNamedFramebuffer                          = nullptr;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC                    glCheckNamedFramebufferStatus                   = nullptr;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC                 glGetNamedFramebufferParameteriv                = nullptr;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC       glGetNamedFramebufferAttachmentParameteriv      = nullptr;
PFNGLCREATERENDERBUFFERSPROC                            glCreateRenderbuffers                           = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC                       glNamedRenderbufferStorage                      = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC            glNamedRenderbufferStorageMultisample           = nullptr;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC                glGetNamedRenderbufferParameteriv               = nullptr;
PFNGLCREATETEXTURESPROC                                 glCreateTextures                                = nullptr;
PFNGLTEXTUREBUFFERPROC                                  glTextureBuffer                                 = nullptr;
PFNGLTEXTUREBUFFERRANGEPROC                             glTextureBufferRange                            = nullptr;
PFNGLTEXTURESTORAGE1DPROC                               glTextureStorage1D                              = nullptr;
PFNGLTEXTURESTORAGE2DPROC                               glTextureStorage2D                              = nullptr;
PFNGLTEXTURESTORAGE3DPROC                               glTextureStorage3D                              = nullptr;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC                    glTextureStorage2DMultisample                   = nullptr;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC                    glTextureStorage3DMultisample                   = nullptr;
PFNGLTEXTURESUBIMAGE1DPROC                              glTextureSubImage1D                             = nullptr;
PFNGLTEXTURESUBIMAGE2DPROC                              glTextureSubImage2D                             = nullptr;
PFNGLTEXTURESUBIMAGE3DPROC                              glTextureSubImage3D                             = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC                    glCompressedTextureSubImage1D                   = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC                    glCompressedTextureSubImage2D                   = nullptr;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC                    glCompressedTextureSubImage3D                   = nullptr;
PFNGLCOPYTEXTURESUBIMAGE1DPROC                          glCopyTextureSubImage1D                         = nullptr;
PFNGLCOPYTEXTURESUBIMAGE2DPROC                          glCopyTextureSubImage2D                         = nullptr;
PFNGLCOPYTEXTURESUBIMAGE3DPROC                          glCopyTextureSubImage3D                         = nullptr;
PFNGLTEXTUREPARAMETERFPROC                              glTextureParameterf                             = nullptr;
PFNGLTEXTUREPARAMETERFVPROC                             glTextureParameterfv                            = nullptr;
PFNGLTEXTUREPARAMETERIPROC                              glTextureParameteri                             = nullptr;
PFNGLTEXTUREPARAMETERIIVPROC                            glTextureParameterIiv                           = nullptr;
PFNGLTEXTUREPARAMETERIUIVPROC                           glTextureParameterIuiv                          = nullptr;
PFNGLTEXTUREPARAMETERIVPROC                             glTextureParameteriv                            = nullptr;
PFNGLGENERATETEXTUREMIPMAPPROC                          glGenerateTextureMipmap                         = nullptr;
PFNGLBINDTEXTUREUNITPROC                                glBindTextureUnit                               = nullptr;
PFNGLGETTEXTUREIMAGEPROC                                glGetTextureImage                               = nullptr;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                      glGetCompressedTextureImage                     = nullptr;
PFNGLGETTEXTURELEVELPARAMETERFVPROC                     glGetTextureLevelParameterfv                    = nullptr;
PFNGLGETTEXTURELEVELPARAMETERIVPROC                     glGetTextureLevelParameteriv                    = nullptr;
PFNGLGETTEXTUREPARAMETERFVPROC                          glGetTextureParameterfv                         = nullptr;
PFNGLGETTEXTUREPARAMETERIIVPROC                         glGetTextureParameterIiv                        = nullptr;
PFNGLGETTEXTUREPARAMETERIUIVPROC                        glGetTextureParameterIuiv                       = nullptr;
PFNGLGETTEXTUREPARAMETERIVPROC                          glGetTextureParameteriv                         = nullptr;
PFNGLCREATEVERTEXARRAYSPROC                             glCreateVertexArrays                            = nullptr;
PFNGLDISABLEVERTEXARRAYATTRIBPROC                       glDisableVertexArrayAttrib                      = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBPROC                        glEnableVertexArrayAttrib                       = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC                       glVertexArrayElementBuffer                      = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC                        glVertexArrayVertexBuffer                       = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERSPROC                       glVertexArrayVertexBuffers                      = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC                        glVertexArrayAttribFormat                       = nullptr;
PFNGLVERTEXARRAYATTRIBIFORMATPROC                       glVertexArrayAttribIFormat                      = nullptr;
PFNGLVERTEXARRAYATTRIBLFORMATPROC                       glVertexArrayAttribLFormat                      = nullptr;
PFNGLVERTEXARRAYATTRIBBINDINGPROC                       glVertexArrayAttribBinding                      = nullptr;
PFNGLVERTEXARRAYBINDINGDIVISORPROC                      glVertexArrayBindingDivisor                     = nullptr;
PFNGLGETVERTEXARRAYIVPROC                               glGetVertexArrayiv                              = nullptr;
PFNGLGETVERTEXARRAYINDEXEDIVPROC                        glGetVertexArrayIndexediv                       = nullptr;
PFNGLGETVERTEXARRAYINDEXED64IVPROC                      glGetVertexArrayIndexed64iv                     = nullptr;
PFNGLCREATESAMPLERSPROC                                 glCreateSamplers                                = nullptr;
PFNGLCREATEPROGRAMPIPELINESPROC                         glCreateProgramPipelines                        = nullptr;
PFNGLCREATEQUERIESPROC                                  glCreateQueries                                 = nullptr;
PFNGLGETQUERYBUFFEROBJECTIVPROC                         glGetQueryBufferObjectiv                        = nullptr;
PFNGLGETQUERYBUFFEROBJECTUIVPROC                        glGetQueryBufferObjectuiv                       = nullptr;
PFNGLGETQUERYBUFFEROBJECTI64VPROC                       glGetQueryBufferObjecti64v                      = nullptr;
PFNGLGETQUERYBUFFEROBJECTUI64VPROC                      glGetQueryBufferObjectui64v                     = nullptr;

#endif // /ifndef(__APPLE__)


} // /namespace LLGL



// ================================================================================
