/*
 * GLExtension.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_EXTENSIONS_H__
#define __LLGL_GL_EXTENSIONS_H__


#include "../OpenGL.h"


namespace LLGL
{


/* --- Platform specific GL extensions --- */

#if defined(_WIN32)

extern PFNWGLSWAPINTERVALEXTPROC                            wglSwapIntervalEXT;
extern PFNWGLCHOOSEPIXELFORMATARBPROC                       wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC                    wglCreateContextAttribsARB;
extern PFNWGLGETEXTENSIONSSTRINGARBPROC                     wglGetExtensionsStringARB;

#elif defined(__linux__)

extern PFNGLXSWAPINTERVALSGIPROC                            glXSwapIntervalSGI;

#endif
    
#ifdef __APPLE__
    
//...
    
#else

#if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

/* --- GL 3.0 extensions (for Core Profile) --- */

extern PFNGLGETSTRINGIPROC                                  glGetStringi;

#endif

/* --- Blending (GL_ARB_draw_buffers_blend) --- */

extern PFNGLBLENDFUNCSEPARATEPROC                           glBlendFuncSeparate;
extern PFNGLBLENDFUNCSEPARATEIPROC                          glBlendFuncSeparatei;

/* --- Blending factor (GL_EXT_blend_color) --- */

extern PFNGLBLENDCOLORPROC                                  glBlendColor;

/* --- Indexed (GL_EXT_draw_buffers2) --- */

extern PFNGLCOLORMASKIPROC                                  glColorMaski;
extern PFNGLGETBOOLEANI_VPROC                               glGetBooleani_v;
extern PFNGLGETINTEGERI_VPROC                               glGetIntegeri_v;
extern PFNGLENABLEIPROC                                     glEnablei;
extern PFNGLDISABLEIPROC                                    glDisablei;
extern PFNGLISENABLEDIPROC                                  glIsEnabledi;

/* --- Multi texture (GL_ARB_multitexture) --- */

extern PFNGLACTIVETEXTUREPROC                               glActiveTexture;

/* --- 3D texture (GL_EXT_texture3D) --- */

extern PFNGLTEXIMAGE3DPROC                                  glTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC                               glTexSubImage3D;

/* --- Clear texture (GL_ARB_clear_texture) --- */

extern PFNGLCLEARTEXIMAGEPROC                               glClearTexImage;
extern PFNGLCLEARTEXSUBIMAGEPROC                            glClearTexSubImage;

/* --- Texture compression (GL_ARB_texture_compression) --- */

extern PFNGLCOMPRESSEDTEXIMAGE1DPROC                        glCompressedTexImage1D;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC                        glCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXIMAGE3DPROC                        glCompressedTexImage3D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                     glCompressedTexSubImage1D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                     glCompressedTexSubImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                     glCompressedTexSubImage3D;
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC                       glGetCompressedTexImage;

/* --- Sampler objects (GL_ARB_sampler_objects) --- */

extern PFNGLGENSAMPLERSPROC                                 glGenSamplers;
extern PFNGLDELETESAMPLERSPROC                              glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC                                 glBindSampler;
extern PFNGLSAMPLERPARAMETERIPROC                           glSamplerParameteri;
extern PFNGLSAMPLERPARAMETERFPROC                           glSamplerParameterf;
extern PFNGLSAMPLERPARAMETERIVPROC                          glSamplerParameteriv;
extern PFNGLSAMPLERPARAMETERFVPROC                          glSamplerParameterfv;

/* --- Multi bind (GL_ARB_multi_bind) --- */

extern PFNGLBINDBUFFERSBASEPROC                             glBindBuffersBase;
extern PFNGLBINDBUFFERSRANGEPROC                            glBindBuffersRange;
extern PFNGLBINDTEXTURESPROC                                glBindTextures;
extern PFNGLBINDSAMPLERSPROC                                glBindSamplers;
extern PFNGLBINDIMAGETEXTURESPROC                           glBindImageTextures;
extern PFNGLBINDVERTEXBUFFERSPROC                           glBindVertexBuffers;

/* --- Vertex buffer object (GL_ARB_vertex_buffer_object) --- */

extern PFNGLGENBUFFERSPROC                                  glGenBuffers;
extern PFNGLDELETEBUFFERSPROC                               glDeleteBuffers;
extern PFNGLBINDBUFFERPROC                                  glBindBuffer;
extern PFNGLBUFFERDATAPROC                                  glBufferData;
extern PFNGLBUFFERSUBDATAPROC                               glBufferSubData;
extern PFNGLMAPBUFFERPROC                                   glMapBuffer;
extern PFNGLUNMAPBUFFERPROC                                 glUnmapBuffer;

/* --- Vertex attributes (GL_ARB_vertex_buffer_object???) --- */

extern PFNGLENABLEVERTEXATTRIBARRAYPROC                     glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC                    glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC                         glVertexAttribPointer;
extern PFNGLVERTEXATTRIBIPOINTERPROC                        glVertexAttribIPointer;
extern PFNGLBINDATTRIBLOCATIONPROC                          glBindAttribLocation;

/* --- Instanced arrays (GL_ARB_instanced_arrays) --- */

extern PFNGLVERTEXATTRIBDIVISORPROC                         glVertexAttribDivisor;

/* --- Draw buffers (GL_ARB_draw_buffers) --- */

extern PFNGLDRAWBUFFERSPROC                                 glDrawBuffers;

/* --- Vertex array objects (GL_ARB_vertex_array_object) --- */

extern PFNGLGENVERTEXARRAYSPROC                             glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC                          glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC                             glBindVertexArray;

/* --- Frame buffer objects (GL_ARB_framebuffer_object) --- */

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

/* --- Instanced drawing (GL_ARB_draw_instanced) --- */

extern PFNGLDRAWARRAYSINSTANCEDPROC                         glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC                       glDrawElementsInstanced;

/* --- Base vertex drawing (GL_ARB_draw_elements_base_vertex) --- */

extern PFNGLDRAWELEMENTSBASEVERTEXPROC                      glDrawElementsBaseVertex;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             glDrawElementsInstancedBaseVertex;

/* --- Instanced offset drawing (GL_ARB_base_instance) --- */

extern PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             glDrawArraysInstancedBaseInstance;
extern PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           glDrawElementsInstancedBaseInstance;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;

/* --- OpenGL shader (GL_ARB_shader_objects) --- */

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

/* --- Tessellation shader (GL_ARB_tessellation_shader) --- */

extern PFNGLPATCHPARAMETERIPROC                             glPatchParameteri;
extern PFNGLPATCHPARAMETERFVPROC                            glPatchParameterfv;

/* --- Compute shader (GL_ARB_compute_shader) --- */

extern PFNGLDISPATCHCOMPUTEPROC                             glDispatchCompute;
extern PFNGLDISPATCHCOMPUTEINDIRECTPROC                     glDispatchComputeIndirect;

/* --- Binary program (GL_ARB_get_program_binary) --- */

extern PFNGLGETPROGRAMBINARYPROC                            glGetProgramBinary;
extern PFNGLPROGRAMBINARYPROC                               glProgramBinary;
extern PFNGLPROGRAMPARAMETERIPROC                           glProgramParameteri;

/* --- Program interface query (GL_ARB_program_interface_query) --- */

extern PFNGLGETPROGRAMINTERFACEIVPROC                       glGetProgramInterfaceiv;
extern PFNGLGETPROGRAMRESOURCEINDEXPROC                     glGetProgramResourceIndex;
extern PFNGLGETPROGRAMRESOURCENAMEPROC                      glGetProgramResourceName;
extern PFNGLGETPROGRAMRESOURCEIVPROC                        glGetProgramResourceiv;
extern PFNGLGETPROGRAMRESOURCELOCATIONPROC                  glGetProgramResourceLocation;
extern PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC             glGetProgramResourceLocationIndex;

/* --- Uniform buffer objects (GL_ARB_uniform_buffer_objects) --- */

extern PFNGLGETUNIFORMBLOCKINDEXPROC                        glGetUniformBlockIndex;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   glGetActiveUniformBlockName;
extern PFNGLUNIFORMBLOCKBINDINGPROC                         glUniformBlockBinding;
extern PFNGLBINDBUFFERBASEPROC                              glBindBufferBase;

/* --- Shader storage buffer objects (GL_ARB_shader_storage_buffer_object) --- */

extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC                   glShaderStorageBlockBinding;

/* --- Query objects (GL_ARB_occlusion_query) --- */

extern PFNGLGENQUERIESPROC                                  glGenQueries;
extern PFNGLDELETEQUERIESPROC                               glDeleteQueries;
extern PFNGLBEGINQUERYPROC                                  glBeginQuery;
extern PFNGLENDQUERYPROC                                    glEndQuery;
extern PFNGLGETQUERYOBJECTIVPROC                            glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC                           glGetQueryObjectuiv;

/* --- Condition render objects (GL_NV_conditional_render) --- */

extern PFNGLBEGINCONDITIONALRENDERPROC                      glBeginConditionalRender;
extern PFNGLENDCONDITIONALRENDERPROC                        glEndConditionalRender;

/* --- Timer query objects (GL_ARB_timer_query) --- */

extern PFNGLQUERYCOUNTERPROC                                glQueryCounter;
extern PFNGLGETQUERYOBJECTI64VPROC                          glGetQueryObjecti64v;
extern PFNGLGETQUERYOBJECTUI64VPROC                         glGetQueryObjectui64v;

/* --- Viewport array (GL_ARB_viewport_array) --- */

extern PFNGLVIEWPORTARRAYVPROC                              glViewportArrayv;
extern PFNGLSCISSORARRAYVPROC                               glScissorArrayv;
extern PFNGLDEPTHRANGEARRAYVPROC                            glDepthRangeArrayv;

/* --- ??? --- */

extern PFNGLSTENCILFUNCSEPARATEPROC                         glStencilFuncSeparate;
extern PFNGLSTENCILMASKSEPARATEPROC                         glStencilMaskSeparate;
extern PFNGLSTENCILOPSEPARATEPROC                           glStencilOpSeparate;

/* --- Debug context (GL_KHR_debug) --- */

extern PFNGLDEBUGMESSAGECALLBACKPROC                        glDebugMessageCallback;

/* --- Clipping control (GL_ARB_clip_control) --- */

extern PFNGLCLIPCONTROLPROC                                 glClipControl;
    
#endif


} // /namespace LLGL


#endif



// ================================================================================
