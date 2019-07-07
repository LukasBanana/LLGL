/*
 * GLExtensions.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_EXTENSIONS_H
#define LLGL_GL_EXTENSIONS_H


#include <LLGL/Platform/Platform.h>
#include "../OpenGL.h"


namespace LLGL
{


/* Platform specific GL extensions */

#if defined(LLGL_OS_WIN32)

extern PFNWGLSWAPINTERVALEXTPROC                            wglSwapIntervalEXT;
extern PFNWGLCHOOSEPIXELFORMATARBPROC                       wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC                    wglCreateContextAttribsARB;
extern PFNWGLGETEXTENSIONSSTRINGARBPROC                     wglGetExtensionsStringARB;

#elif defined(LLGL_OS_LINUX)

extern PFNGLXSWAPINTERVALSGIPROC                            glXSwapIntervalSGI;

#endif

#ifndef LLGL_OS_MACOS

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

/* GL 3.0 extensions (for Core Profile) */

extern PFNGLGETSTRINGIPROC                                  glGetStringi;

#endif

/* GL_EXT_blend_func_separate */

extern PFNGLBLENDFUNCSEPARATEPROC                           glBlendFuncSeparate;

/* GL_EXT_blend_minmax */

extern PFNGLBLENDEQUATIONPROC                               glBlendEquation;

/* GL_EXT_blend_color */

extern PFNGLBLENDCOLORPROC                                  glBlendColor;

/* GL_EXT_blend_equation_separate */

extern PFNGLBLENDEQUATIONSEPARATEPROC                       glBlendEquationSeparate;

/* GL_ARB_draw_buffers */

extern PFNGLDRAWBUFFERSPROC                                 glDrawBuffers;

/* GL_EXT_draw_buffers2 */

extern PFNGLCOLORMASKIPROC                                  glColorMaski;
extern PFNGLGETBOOLEANI_VPROC                               glGetBooleani_v;
extern PFNGLGETINTEGERI_VPROC                               glGetIntegeri_v;
extern PFNGLENABLEIPROC                                     glEnablei;
extern PFNGLDISABLEIPROC                                    glDisablei;
extern PFNGLISENABLEDIPROC                                  glIsEnabledi;

/* GL_ARB_draw_buffers_blend */

extern PFNGLBLENDEQUATIONIPROC                              glBlendEquationi;
extern PFNGLBLENDEQUATIONSEPARATEIPROC                      glBlendEquationSeparatei;
extern PFNGLBLENDFUNCIPROC                                  glBlendFunci;
extern PFNGLBLENDFUNCSEPARATEIPROC                          glBlendFuncSeparatei;

/* GL_ARB_multitexture */

extern PFNGLACTIVETEXTUREPROC                               glActiveTexture;

/* GL_EXT_texture3D */

extern PFNGLTEXIMAGE3DPROC                                  glTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC                               glTexSubImage3D;

/* GL_ARB_clear_texture */

extern PFNGLCLEARTEXIMAGEPROC                               glClearTexImage;
extern PFNGLCLEARTEXSUBIMAGEPROC                            glClearTexSubImage;

/* GL_ARB_texture_compression */

extern PFNGLCOMPRESSEDTEXIMAGE1DPROC                        glCompressedTexImage1D;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC                        glCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXIMAGE3DPROC                        glCompressedTexImage3D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                     glCompressedTexSubImage1D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                     glCompressedTexSubImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                     glCompressedTexSubImage3D;
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC                       glGetCompressedTexImage;

/* GL_ARB_texture_multisample */

extern PFNGLTEXIMAGE2DMULTISAMPLEPROC                       glTexImage2DMultisample;
extern PFNGLTEXIMAGE3DMULTISAMPLEPROC                       glTexImage3DMultisample;
extern PFNGLGETMULTISAMPLEFVPROC                            glGetMultisamplefv;
extern PFNGLSAMPLEMASKIPROC                                 glSampleMaski;

/* GL_ARB_sampler_objects */

extern PFNGLGENSAMPLERSPROC                                 glGenSamplers;
extern PFNGLDELETESAMPLERSPROC                              glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC                                 glBindSampler;
extern PFNGLSAMPLERPARAMETERIPROC                           glSamplerParameteri;
extern PFNGLSAMPLERPARAMETERFPROC                           glSamplerParameterf;
extern PFNGLSAMPLERPARAMETERIVPROC                          glSamplerParameteriv;
extern PFNGLSAMPLERPARAMETERFVPROC                          glSamplerParameterfv;

/* GL_ARB_multi_bind */

extern PFNGLBINDBUFFERSBASEPROC                             glBindBuffersBase;
extern PFNGLBINDBUFFERSRANGEPROC                            glBindBuffersRange;
extern PFNGLBINDTEXTURESPROC                                glBindTextures;
extern PFNGLBINDSAMPLERSPROC                                glBindSamplers;
extern PFNGLBINDIMAGETEXTURESPROC                           glBindImageTextures;
extern PFNGLBINDVERTEXBUFFERSPROC                           glBindVertexBuffers;

/* GL_ARB_vertex_buffer_object */

extern PFNGLGENBUFFERSPROC                                  glGenBuffers;
extern PFNGLDELETEBUFFERSPROC                               glDeleteBuffers;
extern PFNGLBINDBUFFERPROC                                  glBindBuffer;
extern PFNGLISBUFFERPROC                                    glIsBuffer;
extern PFNGLBUFFERDATAPROC                                  glBufferData;
extern PFNGLBUFFERSUBDATAPROC                               glBufferSubData;
extern PFNGLGETBUFFERSUBDATAPROC                            glGetBufferSubData;
extern PFNGLMAPBUFFERPROC                                   glMapBuffer;
extern PFNGLUNMAPBUFFERPROC                                 glUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVPROC                        glGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVPROC                           glGetBufferPointerv;

/* GL_ARB_vertex_shader */

extern PFNGLENABLEVERTEXATTRIBARRAYPROC                     glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC                    glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC                         glVertexAttribPointer;
extern PFNGLBINDATTRIBLOCATIONPROC                          glBindAttribLocation;

/* GL_EXT_gpu_shader4 */

extern PFNGLVERTEXATTRIBIPOINTERPROC                        glVertexAttribIPointer;
extern PFNGLBINDFRAGDATALOCATIONPROC                        glBindFragDataLocation;
extern PFNGLGETFRAGDATALOCATIONPROC                         glGetFragDataLocation;

/* GL_ARB_instanced_arrays */

extern PFNGLVERTEXATTRIBDIVISORPROC                         glVertexAttribDivisor;

/* GL_ARB_vertex_array_object */

extern PFNGLGENVERTEXARRAYSPROC                             glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC                          glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC                             glBindVertexArray;
extern PFNGLISVERTEXARRAYPROC                               glIsVertexArray;

/* GL_ARB_framebuffer_object */

extern PFNGLGENRENDERBUFFERSPROC                            glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC                         glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC                            glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC                         glRenderbufferStorage;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC              glRenderbufferStorageMultisample;
extern PFNGLGENFRAMEBUFFERSPROC                             glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC                          glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC                             glBindFramebuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC                      glCheckFramebufferStatus;
extern PFNGLFRAMEBUFFERTEXTUREPROC                          glFramebufferTexture; // <--- other extension!
extern PFNGLFRAMEBUFFERTEXTURE1DPROC                        glFramebufferTexture1D;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC                        glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC                        glFramebufferTexture3D;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC                     glFramebufferTextureLayer;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC                     glFramebufferRenderbuffer;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC         glGetFramebufferAttachmentParameteriv;
extern PFNGLBLITFRAMEBUFFERPROC                             glBlitFramebuffer;
extern PFNGLGENERATEMIPMAPPROC                              glGenerateMipmap;

#if 1 //WHICH EXTENSION??? (introduced GL 3.0)
extern PFNGLCLEARBUFFERIVPROC                               glClearBufferiv;
extern PFNGLCLEARBUFFERUIVPROC                              glClearBufferuiv;
extern PFNGLCLEARBUFFERFVPROC                               glClearBufferfv;
extern PFNGLCLEARBUFFERFIPROC                               glClearBufferfi;
#endif

/* GL_ARB_draw_instanced */

extern PFNGLDRAWARRAYSINSTANCEDPROC                         glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC                       glDrawElementsInstanced;

/* GL_ARB_draw_elements_base_vertex */

extern PFNGLDRAWELEMENTSBASEVERTEXPROC                      glDrawElementsBaseVertex;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             glDrawElementsInstancedBaseVertex;

/* GL_ARB_base_instance */

extern PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             glDrawArraysInstancedBaseInstance;
extern PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           glDrawElementsInstancedBaseInstance;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;

/* GL_ARB_shader_objects */

extern PFNGLCREATESHADERPROC                                glCreateShader;
extern PFNGLSHADERSOURCEPROC                                glShaderSource;
extern PFNGLCOMPILESHADERPROC                               glCompileShader;
extern PFNGLGETSHADERIVPROC                                 glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC                            glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC                                glDeleteShader;
extern PFNGLCREATEPROGRAMPROC                               glCreateProgram;
extern PFNGLDELETEPROGRAMPROC                               glDeleteProgram;
extern PFNGLATTACHSHADERPROC                                glAttachShader;
extern PFNGLDETACHSHADERPROC                                glDetachShader;
extern PFNGLLINKPROGRAMPROC                                 glLinkProgram;
extern PFNGLVALIDATEPROGRAMPROC                             glValidateProgram;
extern PFNGLGETPROGRAMIVPROC                                glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC                           glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC                                  glUseProgram;
extern PFNGLGETACTIVEATTRIBPROC                             glGetActiveAttrib;
extern PFNGLGETATTRIBLOCATIONPROC                           glGetAttribLocation;
extern PFNGLGETACTIVEUNIFORMARBPROC                         glGetActiveUniform;
extern PFNGLGETUNIFORMLOCATIONPROC                          glGetUniformLocation;
extern PFNGLGETATTACHEDSHADERSPROC                          glGetAttachedShaders;
extern PFNGLUNIFORM1FPROC                                   glUniform1f;
extern PFNGLUNIFORM2FPROC                                   glUniform2f;
extern PFNGLUNIFORM3FPROC                                   glUniform3f;
extern PFNGLUNIFORM4FPROC                                   glUniform4f;
extern PFNGLUNIFORM1IPROC                                   glUniform1i;
extern PFNGLUNIFORM2IPROC                                   glUniform2i;
extern PFNGLUNIFORM3IPROC                                   glUniform3i;
extern PFNGLUNIFORM4IPROC                                   glUniform4i;
extern PFNGLUNIFORM1FVPROC                                  glUniform1fv;
extern PFNGLUNIFORM2FVPROC                                  glUniform2fv;
extern PFNGLUNIFORM3FVPROC                                  glUniform3fv;
extern PFNGLUNIFORM4FVPROC                                  glUniform4fv;
extern PFNGLUNIFORM1IVPROC                                  glUniform1iv;
extern PFNGLUNIFORM2IVPROC                                  glUniform2iv;
extern PFNGLUNIFORM3IVPROC                                  glUniform3iv;
extern PFNGLUNIFORM4IVPROC                                  glUniform4iv;
extern PFNGLUNIFORMMATRIX2FVPROC                            glUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC                            glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC                            glUniformMatrix4fv;
extern PFNGLGETUNIFORMIVPROC                                glGetUniformiv;
extern PFNGLGETUNIFORMFVPROC                                glGetUniformfv;

/* **GL_ARB_shader_objects** TODO: which extension from GL 2.1 ??? */

extern PFNGLUNIFORMMATRIX2X3FVPROC                          glUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC                          glUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC                          glUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC                          glUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC                          glUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC                          glUniformMatrix4x3fv;

/* **GL_ARB_shader_objects** TODO: which extension from GL 3.0 ??? */

extern PFNGLUNIFORM1UIVPROC                                 glUniform1uiv;
extern PFNGLUNIFORM2UIVPROC                                 glUniform2uiv;
extern PFNGLUNIFORM3UIVPROC                                 glUniform3uiv;
extern PFNGLUNIFORM4UIVPROC                                 glUniform4uiv;

/* **GL_ARB_shader_objects** TODO: which extension from GL 4.0 ??? */

extern PFNGLUNIFORM1DVPROC                                  glUniform1dv;
extern PFNGLUNIFORM2DVPROC                                  glUniform2dv;
extern PFNGLUNIFORM3DVPROC                                  glUniform3dv;
extern PFNGLUNIFORM4DVPROC                                  glUniform4dv;
extern PFNGLUNIFORMMATRIX2DVPROC                            glUniformMatrix2dv;
extern PFNGLUNIFORMMATRIX3DVPROC                            glUniformMatrix3dv;
extern PFNGLUNIFORMMATRIX4DVPROC                            glUniformMatrix4dv;
extern PFNGLUNIFORMMATRIX2X3DVPROC                          glUniformMatrix2x3dv;
extern PFNGLUNIFORMMATRIX2X4DVPROC                          glUniformMatrix2x4dv;
extern PFNGLUNIFORMMATRIX3X2DVPROC                          glUniformMatrix3x2dv;
extern PFNGLUNIFORMMATRIX3X4DVPROC                          glUniformMatrix3x4dv;
extern PFNGLUNIFORMMATRIX4X2DVPROC                          glUniformMatrix4x2dv;
extern PFNGLUNIFORMMATRIX4X3DVPROC                          glUniformMatrix4x3dv;

/* GL_ARB_tessellation_shader */

extern PFNGLPATCHPARAMETERIPROC                             glPatchParameteri;
extern PFNGLPATCHPARAMETERFVPROC                            glPatchParameterfv;

/* GL_ARB_compute_shader */

extern PFNGLDISPATCHCOMPUTEPROC                             glDispatchCompute;
extern PFNGLDISPATCHCOMPUTEINDIRECTPROC                     glDispatchComputeIndirect;

/* GL_ARB_get_program_binary */

extern PFNGLGETPROGRAMBINARYPROC                            glGetProgramBinary;
extern PFNGLPROGRAMBINARYPROC                               glProgramBinary;
extern PFNGLPROGRAMPARAMETERIPROC                           glProgramParameteri;

/* GL_ARB_program_interface_query */

extern PFNGLGETPROGRAMINTERFACEIVPROC                       glGetProgramInterfaceiv;
extern PFNGLGETPROGRAMRESOURCEINDEXPROC                     glGetProgramResourceIndex;
extern PFNGLGETPROGRAMRESOURCENAMEPROC                      glGetProgramResourceName;
extern PFNGLGETPROGRAMRESOURCEIVPROC                        glGetProgramResourceiv;
extern PFNGLGETPROGRAMRESOURCELOCATIONPROC                  glGetProgramResourceLocation;
extern PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC             glGetProgramResourceLocationIndex;

/* GL_ARB_uniform_buffer_object */

extern PFNGLGETUNIFORMBLOCKINDEXPROC                        glGetUniformBlockIndex;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   glGetActiveUniformBlockName;
extern PFNGLUNIFORMBLOCKBINDINGPROC                         glUniformBlockBinding;
extern PFNGLBINDBUFFERBASEPROC                              glBindBufferBase;

/* GL_ARB_shader_storage_buffer_object */

extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC                   glShaderStorageBlockBinding;

/* GL_ARB_occlusion_query */

extern PFNGLGENQUERIESPROC                                  glGenQueries;
extern PFNGLDELETEQUERIESPROC                               glDeleteQueries;
extern PFNGLBEGINQUERYPROC                                  glBeginQuery;
extern PFNGLENDQUERYPROC                                    glEndQuery;
extern PFNGLGETQUERYOBJECTIVPROC                            glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC                           glGetQueryObjectuiv;

/* GL_NV_conditional_render */

extern PFNGLBEGINCONDITIONALRENDERPROC                      glBeginConditionalRender;
extern PFNGLENDCONDITIONALRENDERPROC                        glEndConditionalRender;

/* GL_ARB_timer_query */

extern PFNGLQUERYCOUNTERPROC                                glQueryCounter;
extern PFNGLGETQUERYOBJECTI64VPROC                          glGetQueryObjecti64v;
extern PFNGLGETQUERYOBJECTUI64VPROC                         glGetQueryObjectui64v;

/* GL_ARB_viewport_array */

extern PFNGLVIEWPORTARRAYVPROC                              glViewportArrayv;
extern PFNGLSCISSORARRAYVPROC                               glScissorArrayv;
extern PFNGLDEPTHRANGEARRAYVPROC                            glDepthRangeArrayv;

/* GL_ATI_separate_stencil ??? */

extern PFNGLSTENCILFUNCSEPARATEPROC                         glStencilFuncSeparate;
extern PFNGLSTENCILMASKSEPARATEPROC                         glStencilMaskSeparate;
extern PFNGLSTENCILOPSEPARATEPROC                           glStencilOpSeparate;

/* GL_KHR_debug */

extern PFNGLDEBUGMESSAGECALLBACKPROC                        glDebugMessageCallback;

/* GL_ARB_clip_control */

extern PFNGLCLIPCONTROLPROC                                 glClipControl;

/* GL_EXT_transform_feedback */

extern PFNGLBINDBUFFERRANGEPROC                             glBindBufferRange;
extern PFNGLBEGINTRANSFORMFEEDBACKPROC                      glBeginTransformFeedback;
extern PFNGLENDTRANSFORMFEEDBACKPROC                        glEndTransformFeedback;
extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC                   glTransformFeedbackVaryings;
extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC                 glGetTransformFeedbackVarying;

/* GL_NV_transform_feedback */

extern PFNGLBINDBUFFERRANGENVPROC                           glBindBufferRangeNV;
extern PFNGLBEGINTRANSFORMFEEDBACKNVPROC                    glBeginTransformFeedbackNV;
extern PFNGLENDTRANSFORMFEEDBACKNVPROC                      glEndTransformFeedbackNV;
extern PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC                 glTransformFeedbackVaryingsNV;
extern PFNGLGETVARYINGLOCATIONNVPROC                        glGetVaryingLocationNV;
extern PFNGLGETACTIVEVARYINGNVPROC                          glGetActiveVaryingNV;

/* GL_ARB_sync */

extern PFNGLFENCESYNCPROC                                   glFenceSync;
extern PFNGLISSYNCPROC                                      glIsSync;
extern PFNGLDELETESYNCPROC                                  glDeleteSync;
extern PFNGLCLIENTWAITSYNCPROC                              glClientWaitSync;
extern PFNGLWAITSYNCPROC                                    glWaitSync;
extern PFNGLGETINTEGER64VPROC                               glGetInteger64v;
extern PFNGLGETSYNCIVPROC                                   glGetSynciv;

/* GL_ARB_internalformat_query */

extern PFNGLGETINTERNALFORMATIVPROC                         glGetInternalformativ;

/* GL_ARB_internalformat_query2 */

extern PFNGLGETINTERNALFORMATI64VPROC                       glGetInternalformati64v;

/* GL_ARB_ES2_compatibility */

extern PFNGLRELEASESHADERCOMPILERPROC                       glReleaseShaderCompiler;
extern PFNGLSHADERBINARYPROC                                glShaderBinary;
extern PFNGLGETSHADERPRECISIONFORMATPROC                    glGetShaderPrecisionFormat;
extern PFNGLDEPTHRANGEFPROC                                 glDepthRangef;
extern PFNGLCLEARDEPTHFPROC                                 glClearDepthf;

/* GL_ARB_gl_spirv */

extern PFNGLSPECIALIZESHADERPROC                            glSpecializeShader;

/* GL_ARB_texture_storage */

extern PFNGLTEXSTORAGE1DPROC                                glTexStorage1D;
extern PFNGLTEXSTORAGE2DPROC                                glTexStorage2D;
extern PFNGLTEXSTORAGE3DPROC                                glTexStorage3D;

/* GL_ARB_texture_storage_multisample */

extern PFNGLTEXSTORAGE2DMULTISAMPLEPROC                     glTexStorage2DMultisample;
extern PFNGLTEXSTORAGE3DMULTISAMPLEPROC                     glTexStorage3DMultisample;

/* GL_ARB_buffer_storage */

extern PFNGLBUFFERSTORAGEPROC                               glBufferStorage;

/* ARB_copy_buffer */

extern PFNGLCOPYBUFFERSUBDATAPROC                           glCopyBufferSubData;

/* GL_ARB_polygon_offset_clamp */

extern PFNGLPOLYGONOFFSETCLAMPPROC                          glPolygonOffsetClamp;

/* GL_ARB_texture_view */

extern PFNGLTEXTUREVIEWPROC                                 glTextureView;

/* GL_ARB_shader_image_load_store */

extern PFNGLBINDIMAGETEXTUREPROC                            glBindImageTexture;
extern PFNGLMEMORYBARRIERPROC                               glMemoryBarrier;

/* GL_ARB_framebuffer_no_attachments */

extern PFNGLFRAMEBUFFERPARAMETERIPROC                       glFramebufferParameteri;
extern PFNGLGETFRAMEBUFFERPARAMETERIVPROC                   glGetFramebufferParameteriv;

/* GL_ARB_clear_buffer_object */

extern PFNGLCLEARBUFFERDATAPROC                             glClearBufferData;
extern PFNGLCLEARBUFFERSUBDATAPROC                          glClearBufferSubData;

/* GL_ARB_draw_indirect */

extern PFNGLDRAWARRAYSINDIRECTPROC                          glDrawArraysIndirect;
extern PFNGLDRAWELEMENTSINDIRECTPROC                        glDrawElementsIndirect;

/* GL_ARB_multi_draw_indirect */

extern PFNGLMULTIDRAWARRAYSINDIRECTPROC                     glMultiDrawArraysIndirect;
extern PFNGLMULTIDRAWELEMENTSINDIRECTPROC                   glMultiDrawElementsIndirect;

/* GL_ARB_direct_state_access */

extern PFNGLCREATETRANSFORMFEEDBACKSPROC                    glCreateTransformFeedbacks;
extern PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC                 glTransformFeedbackBufferBase;
extern PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC                glTransformFeedbackBufferRange;
extern PFNGLGETTRANSFORMFEEDBACKIVPROC                      glGetTransformFeedbackiv;
extern PFNGLGETTRANSFORMFEEDBACKI_VPROC                     glGetTransformFeedbacki_v;
extern PFNGLGETTRANSFORMFEEDBACKI64_VPROC                   glGetTransformFeedbacki64_v;
extern PFNGLCREATEBUFFERSPROC                               glCreateBuffers;
extern PFNGLNAMEDBUFFERSTORAGEPROC                          glNamedBufferStorage;
extern PFNGLNAMEDBUFFERDATAPROC                             glNamedBufferData;
extern PFNGLNAMEDBUFFERSUBDATAPROC                          glNamedBufferSubData;
extern PFNGLCOPYNAMEDBUFFERSUBDATAPROC                      glCopyNamedBufferSubData;
extern PFNGLCLEARNAMEDBUFFERDATAPROC                        glClearNamedBufferData;
extern PFNGLCLEARNAMEDBUFFERSUBDATAPROC                     glClearNamedBufferSubData;
extern PFNGLMAPNAMEDBUFFERPROC                              glMapNamedBuffer;
extern PFNGLMAPNAMEDBUFFERRANGEPROC                         glMapNamedBufferRange;
extern PFNGLUNMAPNAMEDBUFFERPROC                            glUnmapNamedBuffer;
extern PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC                 glFlushMappedNamedBufferRange;
extern PFNGLGETNAMEDBUFFERPARAMETERIVPROC                   glGetNamedBufferParameteriv;
extern PFNGLGETNAMEDBUFFERPARAMETERI64VPROC                 glGetNamedBufferParameteri64v;
extern PFNGLGETNAMEDBUFFERPOINTERVPROC                      glGetNamedBufferPointerv;
extern PFNGLGETNAMEDBUFFERSUBDATAPROC                       glGetNamedBufferSubData;
extern PFNGLCREATEFRAMEBUFFERSPROC                          glCreateFramebuffers;
extern PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC                glNamedFramebufferRenderbuffer;
extern PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC                  glNamedFramebufferParameteri;
extern PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                     glNamedFramebufferTexture;
extern PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC                glNamedFramebufferTextureLayer;
extern PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC                  glNamedFramebufferDrawBuffer;
extern PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC                 glNamedFramebufferDrawBuffers;
extern PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC                  glNamedFramebufferReadBuffer;
extern PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC              glInvalidateNamedFramebufferData;
extern PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC           glInvalidateNamedFramebufferSubData;
extern PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                     glClearNamedFramebufferiv;
extern PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                    glClearNamedFramebufferuiv;
extern PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                     glClearNamedFramebufferfv;
extern PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                     glClearNamedFramebufferfi;
extern PFNGLBLITNAMEDFRAMEBUFFERPROC                        glBlitNamedFramebuffer;
extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC                 glCheckNamedFramebufferStatus;
extern PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC              glGetNamedFramebufferParameteriv;
extern PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC    glGetNamedFramebufferAttachmentParameteriv;
extern PFNGLCREATERENDERBUFFERSPROC                         glCreateRenderbuffers;
extern PFNGLNAMEDRENDERBUFFERSTORAGEPROC                    glNamedRenderbufferStorage;
extern PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC         glNamedRenderbufferStorageMultisample;
extern PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC             glGetNamedRenderbufferParameteriv;
extern PFNGLCREATETEXTURESPROC                              glCreateTextures;
extern PFNGLTEXTUREBUFFERPROC                               glTextureBuffer;
extern PFNGLTEXTUREBUFFERRANGEPROC                          glTextureBufferRange;
extern PFNGLTEXTURESTORAGE1DPROC                            glTextureStorage1D;
extern PFNGLTEXTURESTORAGE2DPROC                            glTextureStorage2D;
extern PFNGLTEXTURESTORAGE3DPROC                            glTextureStorage3D;
extern PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC                 glTextureStorage2DMultisample;
extern PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC                 glTextureStorage3DMultisample;
extern PFNGLTEXTURESUBIMAGE1DPROC                           glTextureSubImage1D;
extern PFNGLTEXTURESUBIMAGE2DPROC                           glTextureSubImage2D;
extern PFNGLTEXTURESUBIMAGE3DPROC                           glTextureSubImage3D;
extern PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC                 glCompressedTextureSubImage1D;
extern PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC                 glCompressedTextureSubImage2D;
extern PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC                 glCompressedTextureSubImage3D;
extern PFNGLCOPYTEXTURESUBIMAGE1DPROC                       glCopyTextureSubImage1D;
extern PFNGLCOPYTEXTURESUBIMAGE2DPROC                       glCopyTextureSubImage2D;
extern PFNGLCOPYTEXTURESUBIMAGE3DPROC                       glCopyTextureSubImage3D;
extern PFNGLTEXTUREPARAMETERFPROC                           glTextureParameterf;
extern PFNGLTEXTUREPARAMETERFVPROC                          glTextureParameterfv;
extern PFNGLTEXTUREPARAMETERIPROC                           glTextureParameteri;
extern PFNGLTEXTUREPARAMETERIIVPROC                         glTextureParameterIiv;
extern PFNGLTEXTUREPARAMETERIUIVPROC                        glTextureParameterIuiv;
extern PFNGLTEXTUREPARAMETERIVPROC                          glTextureParameteriv;
extern PFNGLGENERATETEXTUREMIPMAPPROC                       glGenerateTextureMipmap;
extern PFNGLBINDTEXTUREUNITPROC                             glBindTextureUnit;
extern PFNGLGETTEXTUREIMAGEPROC                             glGetTextureImage;
extern PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                   glGetCompressedTextureImage;
extern PFNGLGETTEXTURELEVELPARAMETERFVPROC                  glGetTextureLevelParameterfv;
extern PFNGLGETTEXTURELEVELPARAMETERIVPROC                  glGetTextureLevelParameteriv;
extern PFNGLGETTEXTUREPARAMETERFVPROC                       glGetTextureParameterfv;
extern PFNGLGETTEXTUREPARAMETERIIVPROC                      glGetTextureParameterIiv;
extern PFNGLGETTEXTUREPARAMETERIUIVPROC                     glGetTextureParameterIuiv;
extern PFNGLGETTEXTUREPARAMETERIVPROC                       glGetTextureParameteriv;
extern PFNGLCREATEVERTEXARRAYSPROC                          glCreateVertexArrays;
extern PFNGLDISABLEVERTEXARRAYATTRIBPROC                    glDisableVertexArrayAttrib;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC                     glEnableVertexArrayAttrib;
extern PFNGLVERTEXARRAYELEMENTBUFFERPROC                    glVertexArrayElementBuffer;
extern PFNGLVERTEXARRAYVERTEXBUFFERPROC                     glVertexArrayVertexBuffer;
extern PFNGLVERTEXARRAYVERTEXBUFFERSPROC                    glVertexArrayVertexBuffers;
extern PFNGLVERTEXARRAYATTRIBFORMATPROC                     glVertexArrayAttribFormat;
extern PFNGLVERTEXARRAYATTRIBIFORMATPROC                    glVertexArrayAttribIFormat;
extern PFNGLVERTEXARRAYATTRIBLFORMATPROC                    glVertexArrayAttribLFormat;
extern PFNGLVERTEXARRAYATTRIBBINDINGPROC                    glVertexArrayAttribBinding;
extern PFNGLVERTEXARRAYBINDINGDIVISORPROC                   glVertexArrayBindingDivisor;
extern PFNGLGETVERTEXARRAYIVPROC                            glGetVertexArrayiv;
extern PFNGLGETVERTEXARRAYINDEXEDIVPROC                     glGetVertexArrayIndexediv;
extern PFNGLGETVERTEXARRAYINDEXED64IVPROC                   glGetVertexArrayIndexed64iv;
extern PFNGLCREATESAMPLERSPROC                              glCreateSamplers;
extern PFNGLCREATEPROGRAMPIPELINESPROC                      glCreateProgramPipelines;
extern PFNGLCREATEQUERIESPROC                               glCreateQueries;
extern PFNGLGETQUERYBUFFEROBJECTIVPROC                      glGetQueryBufferObjectiv;
extern PFNGLGETQUERYBUFFEROBJECTUIVPROC                     glGetQueryBufferObjectuiv;
extern PFNGLGETQUERYBUFFEROBJECTI64VPROC                    glGetQueryBufferObjecti64v;
extern PFNGLGETQUERYBUFFEROBJECTUI64VPROC                   glGetQueryBufferObjectui64v;

#endif // /ifndef(__APPLE__)


} // /namespace LLGL


#endif



// ================================================================================
