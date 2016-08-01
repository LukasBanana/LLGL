/*
 * GLExtension.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_EXTENSIONS_H__
#define __LLGL_GL_EXTENSIONS_H__


#include <GL/gl.h>
#include <GL/glext.h>

#if defined(WIN32)
#   include <Windows.h>
#   include <GL/wglext.h>
#elif defined(__linux__)
#   include <sys/utsname.h>
#   include <GL/glx.h>
#elif defined(__APPLE__)
#   include <OpenGL/gl.h>
#endif


namespace LLGL
{


struct GLExtensions
{

    /* --- Platform specific GL extensions --- */

    #if defined(FORK_WINDOWS_PLATFORM)

    PFNWGLSWAPINTERVALEXTPROC                    WGLSwapIntervalEXT;
    PFNWGLCHOOSEPIXELFORMATARBPROC               WGLChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC            WGLCreateContextAttribsARB;
    PFNWGLGETEXTENSIONSSTRINGARBPROC             WGLGetExtensionsStringARB;

    #elif defined(FORK_POSIX)

    PFNGLXSWAPINTERVALSGIPROC                    GLXSwapIntervalSGI;

    #endif

    #if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)

    /* --- GL 3.0 extensions (for Core Profile) --- */

    PFNGLGETSTRINGIPROC                                  GetStringi;

    #endif

    /* --- Blending (GL_ARB_draw_buffers_blend) --- */

    PFNGLBLENDFUNCSEPARATEPROC                           BlendFuncSeparate;
    PFNGLBLENDFUNCSEPARATEIPROC                          BlendFuncSeparatei;

    /* --- Multi Texture (GL_ARB_multitexture) --- */

    PFNGLACTIVETEXTUREPROC                               ActiveTexture;
    PFNGLTEXIMAGE3DPROC                                  TexImage3D;
    PFNGLTEXSUBIMAGE3DPROC                               TexSubImage3D;

    /* --- Clear Texture (GL_ARB_clear_texture) --- */

    PFNGLCLEARTEXIMAGEPROC                               ClearTexImage;
    PFNGLCLEARTEXSUBIMAGEPROC                            ClearTexSubImage;

    /* --- Sampler objects (GL_ARB_sampler_objects) --- */

    PFNGLGENSAMPLERSPROC                                 GenSamplers;
    PFNGLDELETESAMPLERSPROC                              DeleteSamplers;
    PFNGLBINDSAMPLERPROC                                 BindSampler;
    PFNGLSAMPLERPARAMETERIPROC                           SamplerParameteri;
    PFNGLSAMPLERPARAMETERFPROC                           SamplerParameterf;
    PFNGLSAMPLERPARAMETERIVPROC                          SamplerParameteriv;
    PFNGLSAMPLERPARAMETERFVPROC                          SamplerParameterfv;

    /* --- Multi bind (GL_ARB_multi_bind) --- */

    PFNGLBINDBUFFERSBASEPROC                             BindBuffersBase;
    PFNGLBINDBUFFERSRANGEPROC                            BindBuffersRange;
    PFNGLBINDTEXTURESPROC                                BindTextures;
    PFNGLBINDSAMPLERSPROC                                BindSamplers;
    PFNGLBINDIMAGETEXTURESPROC                           BindImageTextures;
    PFNGLBINDVERTEXBUFFERSPROC                           BindVertexBuffers;

    /* --- Vertex buffer object (GL_ARB_vertex_buffer_object) --- */

    PFNGLGENBUFFERSPROC                                  GenBuffers;
    PFNGLDELETEBUFFERSPROC                               DeleteBuffers;
    PFNGLBINDBUFFERPROC                                  BindBuffer;
    PFNGLBUFFERDATAPROC                                  BufferData;
    PFNGLBUFFERSUBDATAPROC                               BufferSubData;
    PFNGLMAPBUFFERPROC                                   MapBuffer;
    PFNGLUNMAPBUFFERPROC                                 UnmapBuffer;

    /* --- Vertex attributes (GL_ARB_vertex_buffer_object???) --- */

    PFNGLENABLEVERTEXATTRIBARRAYPROC                     EnableVertexAttribArray;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC                    DisableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC                         VertexAttribPointer;
    PFNGLBINDATTRIBLOCATIONPROC                          BindAttribLocation;

    /* --- Draw buffers (GL_ARB_draw_buffers) --- */

    PFNGLDRAWBUFFERSPROC                                 DrawBuffers;

    /* --- Vertex array objects (GL_ARB_vertex_array_object) --- */

    PFNGLGENVERTEXARRAYSPROC                             GenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC                          DeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC                             BindVertexArray;

    /* --- Frame buffer objects (GL_ARB_framebuffer_object) --- */

    PFNGLGENRENDERBUFFERSPROC                            GenRenderbuffers;
    PFNGLDELETERENDERBUFFERSPROC                         DeleteRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC                            BindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC                         RenderbufferStorage;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC              RenderbufferStorageMultisample;

    PFNGLGENFRAMEBUFFERSPROC                             GenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC                          DeleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC                             BindFramebuffer;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC                      CheckFramebufferStatus;

    PFNGLFRAMEBUFFERTEXTUREPROC                          FramebufferTexture;
    PFNGLFRAMEBUFFERTEXTURE1DPROC                        FramebufferTexture1D;
    PFNGLFRAMEBUFFERTEXTURE2DPROC                        FramebufferTexture2D;
    PFNGLFRAMEBUFFERTEXTURE3DPROC                        FramebufferTexture3D;
    PFNGLFRAMEBUFFERTEXTURELAYERPROC                     FramebufferTextureLayer;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC                     FramebufferRenderbuffer;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC         GetFramebufferAttachmentParameteriv;
    PFNGLBLITFRAMEBUFFERPROC                             BlitFramebuffer;

    PFNGLGENERATEMIPMAPPROC                              GenerateMipmap;

    /* --- Instanced drawing (GL_ARB_draw_instanced) --- */

    PFNGLDRAWARRAYSINSTANCEDPROC                         DrawArraysInstanced;
    PFNGLDRAWELEMENTSINSTANCEDPROC                       DrawElementsInstanced;

    /* --- Base vertex drawing (GL_ARB_draw_elements_base_vertex) --- */

    PFNGLDRAWELEMENTSBASEVERTEXPROC                      DrawElementsBaseVertex;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             DrawElementsInstancedBaseVertex;

    /* --- Instanced offset drawing (GL_ARB_base_instance) --- */

    PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             DrawArraysInstancedBaseInstance;
    PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           DrawElementsInstancedBaseInstance;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC DrawElementsInstancedBaseVertexBaseInstance;

    /* --- OpenGL shader (GL_ARB_shader_objects) --- */

    PFNGLCREATESHADERPROC                                CreateShader;
    PFNGLSHADERSOURCEPROC                                glShaderSource;
    PFNGLCOMPILESHADERPROC                               CompileShader;
    PFNGLGETSHADERIVPROC                                 GetShaderiv;
    PFNGLGETSHADERINFOLOGPROC                            GetShaderInfoLog;
    PFNGLDELETESHADERPROC                                DeleteShader;

    PFNGLCREATEPROGRAMPROC                               CreateProgram;
    PFNGLDELETEPROGRAMPROC                               DeleteProgram;
    PFNGLATTACHSHADERPROC                                AttachShader;
    PFNGLDETACHSHADERPROC                                DetachShader;
    PFNGLLINKPROGRAMPROC                                 LinkProgram;
    PFNGLVALIDATEPROGRAMPROC                             ValidateProgram;
    PFNGLGETPROGRAMIVPROC                                GetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC                           GetProgramInfoLog;
    PFNGLUSEPROGRAMPROC                                  UseProgram;

    PFNGLGETACTIVEATTRIBPROC                             GetActiveAttrib;
    PFNGLGETATTRIBLOCATIONPROC                           GetAttribLocation;

    /* --- Tessellation shader (GL_ARB_tessellation_shader) --- */

    PFNGLPATCHPARAMETERIPROC                             PatchParameteri;
    PFNGLPATCHPARAMETERFVPROC                            PatchParameterfv;

    /* --- Compute shader (GL_ARB_compute_shader) --- */

    PFNGLDISPATCHCOMPUTEPROC                             DispatchCompute;
    PFNGLDISPATCHCOMPUTEINDIRECTPROC                     DispatchComputeIndirect;

    /* --- Binary program (GL_ARB_get_program_binary) --- */

    PFNGLGETPROGRAMBINARYPROC                            GetProgramBinary;
    PFNGLPROGRAMBINARYPROC                               ProgramBinary;
    PFNGLPROGRAMPARAMETERIPROC                           ProgramParameteri;

    /* --- Program interface query (GL_ARB_program_interface_query) --- */

    PFNGLGETPROGRAMINTERFACEIVPROC                       GetProgramInterfaceiv;
    PFNGLGETPROGRAMRESOURCEINDEXPROC                     GetProgramResourceIndex;
    PFNGLGETPROGRAMRESOURCENAMEPROC                      GetProgramResourceName;
    PFNGLGETPROGRAMRESOURCEIVPROC                        GetProgramResourceiv;
    PFNGLGETPROGRAMRESOURCELOCATIONPROC                  GetProgramResourceLocation;
    PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC             GetProgramResourceLocationIndex;

    /* --- Uniform buffer objects (GL_ARB_uniform_buffer_objects) --- */

    PFNGLGETUNIFORMBLOCKINDEXPROC                        GetUniformBlockIndex;
    PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     GetActiveUniformBlockiv;
    PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   GetActiveUniformBlockName;
    PFNGLUNIFORMBLOCKBINDINGPROC                         UniformBlockBinding;
    PFNGLBINDBUFFERBASEPROC                              BindBufferBase;

    /* --- Shader storage buffer objects (GL_ARB_shader_storage_buffer_object) --- */

    PFNGLSHADERSTORAGEBLOCKBINDINGPROC                   ShaderStorageBlockBinding;

    /* --- Query objects (GL_ARB_occlusion_query) --- */

    PFNGLGENQUERIESPROC                                  GenQueries;
    PFNGLDELETEQUERIESPROC                               DeleteQueries;
    PFNGLBEGINQUERYPROC                                  BeginQuery;
    PFNGLENDQUERYPROC                                    EndQuery;
    PFNGLGETQUERYOBJECTIVPROC                            GetQueryObjectiv;
    PFNGLGETQUERYOBJECTUIVPROC                           GetQueryObjectuiv;

    /* --- Viewport array (GL_ARB_viewport_array) --- */

    PFNGLVIEWPORTARRAYVPROC                              ViewportArrayv;
    PFNGLSCISSORARRAYVPROC                               ScissorArrayv;
    PFNGLDEPTHRANGEARRAYVPROC                            DepthRangeArrayv;

    /* --- ??? --- */

    PFNGLSTENCILFUNCSEPARATEPROC                         StencilFuncSeparate;
    PFNGLSTENCILMASKSEPARATEPROC                         StencilMaskSeparate;
    PFNGLSTENCILOPSEPARATEPROC                           StencilOpSeparate;

    /* --- Debug context (GL_KHR_debug) --- */

    PFNGLDEBUGMESSAGECALLBACKPROC                        DebugMessageCallback;
};


} // /namespace LLGL


#endif



// ================================================================================
