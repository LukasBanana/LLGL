/*
 * GLModuleInterface.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionLoader.h"
#include "GLExtensions.h"
#include "GLExtensionsNull.h"
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
    Log::StdErr() << "OS not supported for loading OpenGL extensions" << std::endl;
    return false;
    #endif
    
    /* Check for errors */
    if (!procAddr)
    {
        Log::StdErr() << "failed to load OpenGL procedure: " << procName << std::endl;
        return false;
    }
    
    return true;
}

static void ExtractExtensionsFromString(std::map<std::string, bool>& extMap, const std::string& extString)
{
    size_t first = 0, last = 0;
    
    /* Find next extension name in string */
    while ( ( last = extString.find(' ', first) ) != std::string::npos )
    {
        /* Store current extension name in hash-map */
        auto name = extString.substr(first, last - first);
        extMap[name] = true;
        first = last + 1;
    }
}


#ifndef __APPLE__

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS

#define LOAD_GLPROC(NAME)               \
    if (usePlaceHolder)                 \
        NAME = Dummy_##NAME;            \
    else if (!LoadGLProc(NAME, #NAME))  \
        return false

#else

#define LOAD_GLPROC(NAME)           \
    if (!LoadGLProc(NAME, #NAME))   \
        return false

#endif

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

/* --- Hardware buffer extensions --- */

static bool LoadVBOProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenBuffers    );
    LOAD_GLPROC( glDeleteBuffers );
    LOAD_GLPROC( glBindBuffer    );
    LOAD_GLPROC( glBufferData    );
    LOAD_GLPROC( glBufferSubData );
    LOAD_GLPROC( glMapBuffer     );
    LOAD_GLPROC( glUnmapBuffer   );
    return true;
}

static bool LoadVAOProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenVertexArrays    );
    LOAD_GLPROC( glDeleteVertexArrays );
    LOAD_GLPROC( glBindVertexArray    );
    return true;
}

static bool LoadFBOProcs(bool usePlaceHolder)
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

    LOAD_GLPROC( glFramebufferTexture                  );
    LOAD_GLPROC( glFramebufferTexture1D                );
    LOAD_GLPROC( glFramebufferTexture2D                );
    LOAD_GLPROC( glFramebufferTexture3D                );
    LOAD_GLPROC( glFramebufferTextureLayer             );
    LOAD_GLPROC( glFramebufferRenderbuffer             );
    LOAD_GLPROC( glGetFramebufferAttachmentParameteriv );
    LOAD_GLPROC( glBlitFramebuffer                     );

    LOAD_GLPROC( glGenerateMipmap                      );
    return true;
}

static bool LoadUBOProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetUniformBlockIndex      );
    LOAD_GLPROC( glGetActiveUniformBlockiv   );
    LOAD_GLPROC( glGetActiveUniformBlockName );
    LOAD_GLPROC( glUniformBlockBinding       );
    LOAD_GLPROC( glBindBufferBase            );
    return true;
}

static bool LoadSSBOProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glShaderStorageBlockBinding );
    return true;
}

/* --- Drawing extensions --- */

static bool LoadDrawBuffersProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawBuffers );
    return true;
}

static bool LoadInstancedProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawArraysInstanced   );
    LOAD_GLPROC( glDrawElementsInstanced );
    return true;
}

static bool LoadInstancedOffsetProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawArraysInstancedBaseInstance             );
    LOAD_GLPROC( glDrawElementsInstancedBaseInstance           );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertexBaseInstance );
    return true;
}

static bool LoadBaseVertexProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDrawElementsBaseVertex          );
    LOAD_GLPROC( glDrawElementsInstancedBaseVertex );
    return true;
}

/* --- Shader extensions --- */

static bool LoadShaderProcs(bool usePlaceHolder)
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
    return true;
}

static bool LoadVertexAttribProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glEnableVertexAttribArray  );
    LOAD_GLPROC( glDisableVertexAttribArray );
    LOAD_GLPROC( glVertexAttribPointer      );
    LOAD_GLPROC( glVertexAttribIPointer     );
    LOAD_GLPROC( glBindAttribLocation       );
    return true;
}

static bool LoadTessellationShaderProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glPatchParameteri  );
    LOAD_GLPROC( glPatchParameterfv );
    return true;
}

static bool LoadComputeShaderProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}

static bool LoadProgramBinaryProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetProgramBinary  );
    LOAD_GLPROC( glProgramBinary     );
    LOAD_GLPROC( glProgramParameteri );
    return true;
}

static bool LoadProgramInterfaceQueryProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGetProgramInterfaceiv           );
    LOAD_GLPROC( glGetProgramResourceIndex         );
    LOAD_GLPROC( glGetProgramResourceName          );
    LOAD_GLPROC( glGetProgramResourceiv            );
    LOAD_GLPROC( glGetProgramResourceLocation      );
    LOAD_GLPROC( glGetProgramResourceLocationIndex );
    return true;
}

/* --- Texture extensions --- */

static bool LoadMultiTextureProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glActiveTexture );
    return true;
}

static bool Load3DTextureProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glTexImage3D    );
    LOAD_GLPROC( glTexSubImage3D );
    return true;
}

static bool LoadClearTextureProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glClearTexImage    );
    LOAD_GLPROC( glClearTexSubImage );
    return true;
}

static bool LoadCompressedTextureProcs(bool usePlaceHolder)
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

static bool LoadSamplerProcs(bool usePlaceHolder)
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

static bool LoadQueryObjectProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glGenQueries        );
    LOAD_GLPROC( glDeleteQueries     );
    LOAD_GLPROC( glBeginQuery        );
    LOAD_GLPROC( glEndQuery          );
    LOAD_GLPROC( glGetQueryObjectiv  );
    LOAD_GLPROC( glGetQueryObjectuiv );
    return true;
}

static bool LoadConditionalRenderProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glBeginConditionalRender );
    LOAD_GLPROC( glEndConditionalRender   );
    return true;
}

static bool LoadTimerQueryObjectProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glQueryCounter        );
    LOAD_GLPROC( glGetQueryObjecti64v  );
    LOAD_GLPROC( glGetQueryObjectui64v );
    return true;
}

static bool LoadViewportArrayProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glViewportArrayv   );
    LOAD_GLPROC( glScissorArrayv    );
    LOAD_GLPROC( glDepthRangeArrayv );
    return true;
}

static bool LoadDrawBuffersBlendProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glBlendFuncSeparate  );
    LOAD_GLPROC( glBlendFuncSeparatei );
    return true;
}

static bool LoadMultiBindProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glBindBuffersBase   );
    LOAD_GLPROC( glBindBuffersRange  );
    LOAD_GLPROC( glBindTextures      );
    LOAD_GLPROC( glBindSamplers      );
    LOAD_GLPROC( glBindImageTextures );
    LOAD_GLPROC( glBindVertexBuffers );
    return true;
}

static bool LoadStencilSeparateProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glStencilFuncSeparate );
    LOAD_GLPROC( glStencilMaskSeparate );
    LOAD_GLPROC( glStencilOpSeparate   );
    return true;
}

static bool LoadDebugProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glDebugMessageCallback );
    return true;
}

static bool LoadClipControlProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glClipControl );
    return true;
}

static bool LoadIndexedProcs(bool usePlaceHolder)
{
    LOAD_GLPROC( glColorMaski    );
    LOAD_GLPROC( glGetBooleani_v );
    LOAD_GLPROC( glGetIntegeri_v );
    LOAD_GLPROC( glEnablei       );
    LOAD_GLPROC( glDisablei      );
    LOAD_GLPROC( glIsEnabledi    );
    return true;
}

#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC
    
#endif


/* --- Common extension loading functions --- */

OpenGLExtensionMap QueryExtensions(bool coreProfile)
{
    OpenGLExtensionMap extMap;

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
                    extMap[std::string(extString)] = true;
            }
        }
        
        #endif
    }
    else
    {
        /* Get complete extension string */
        extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extString)
            ExtractExtensionsFromString(extMap, extString);
    }

    #if defined(_WIN32) && defined(WGL_ARB_extensions_string)

    /* Filter Win32 related extensions */
    if (wglGetExtensionsStringARB || LoadGLProc(wglGetExtensionsStringARB, "wglGetExtensionsStringARB"))
    {
        extString = wglGetExtensionsStringARB(wglGetCurrentDC());
        if (extString)
            ExtractExtensionsFromString(extMap, extString);
    }
    
    #endif

    return extMap;
}

void LoadAllExtensions(OpenGLExtensionMap& extMap)
{
    #ifndef __APPLE__
    
    /* Only load GL extensions once */
    static bool extAlreadyLoaded;

    if (extAlreadyLoaded)
        return;

    /* Internal extension loading lambda function */
    auto LoadExtension = [&](const std::string& extName, const std::function<bool(bool)>& extLoadingProc) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extMap.find(extName);
        if (it != extMap.end() && !extLoadingProc(false))
        {
            Log::StdErr() << "failed to load OpenGL extension: " << extName << std::endl;
            it->second = false;
        }
        #ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS
        else if (it == extMap.end())
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(true);
        }
        #endif
    };

    /* Load hardware buffer extensions */
    LoadExtension( "GL_ARB_vertex_buffer_object",         LoadVBOProcs                   );
    LoadExtension( "GL_ARB_vertex_array_object",          LoadVAOProcs                   );
    LoadExtension( "GL_ARB_framebuffer_object",           LoadFBOProcs                   );
    LoadExtension( "GL_ARB_uniform_buffer_object",        LoadUBOProcs                   );
    LoadExtension( "GL_ARB_shader_storage_buffer_object", LoadSSBOProcs                  );

    /* Load drawing extensions */
    LoadExtension( "GL_ARB_draw_buffers",                 LoadDrawBuffersProcs           );
    LoadExtension( "GL_ARB_draw_instanced",               LoadInstancedProcs             );
    LoadExtension( "GL_ARB_base_instance",                LoadInstancedOffsetProcs       );
    LoadExtension( "GL_ARB_draw_elements_base_vertex",    LoadBaseVertexProcs            );

    /* Load shader extensions */
    LoadExtension( "GL_ARB_shader_objects",               LoadShaderProcs                );
    LoadExtension( "GL_ARB_vertex_buffer_object",         LoadVertexAttribProcs          ); // <--- correct extension ???
    LoadExtension( "GL_ARB_tessellation_shader",          LoadTessellationShaderProcs    );
    LoadExtension( "GL_ARB_compute_shader",               LoadComputeShaderProcs         );
    LoadExtension( "GL_ARB_get_program_binary",           LoadProgramBinaryProcs         );
    LoadExtension( "GL_ARB_program_interface_query",      LoadProgramInterfaceQueryProcs );

    /* Load texture extensions */
    LoadExtension( "GL_ARB_multitexture",                 LoadMultiTextureProcs          );
    LoadExtension( "GL_EXT_texture3D",                    Load3DTextureProcs             );
    LoadExtension( "GL_ARB_clear_texture",                LoadClearTextureProcs          );
    LoadExtension( "GL_ARB_texture_compression",          LoadCompressedTextureProcs     );
    LoadExtension( "GL_ARB_sampler_objects",              LoadSamplerProcs               );

    /* Load misc extensions */
    LoadExtension( "GL_ARB_viewport_array",               LoadViewportArrayProcs         );
    LoadExtension( "GL_ARB_draw_buffers_blend",           LoadDrawBuffersBlendProcs      );
    LoadExtension( "GL_ARB_occlusion_query",              LoadQueryObjectProcs           );
    LoadExtension( "GL_NV_conditional_render",            LoadConditionalRenderProcs     );
    LoadExtension( "GL_ARB_timer_query",                  LoadTimerQueryObjectProcs      );
    LoadExtension( "GL_ARB_multi_bind",                   LoadMultiBindProcs             );
    LoadExtension( "GL_EXT_stencil_two_side",             LoadStencilSeparateProcs       ); // <--- correct extension ???
    LoadExtension( "GL_KHR_debug",                        LoadDebugProcs                 );
    LoadExtension( "GL_ARB_clip_control",                 LoadClipControlProcs           );
    LoadExtension( "GL_EXT_draw_buffers2",                LoadIndexedProcs               );

    extAlreadyLoaded = true;
    
    #endif
}


} // /namespace LLGL



// ================================================================================
