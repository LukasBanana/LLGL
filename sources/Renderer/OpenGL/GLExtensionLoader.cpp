/*
 * GLModuleInterface.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#if 0

#include "GLExtensionLoader.h"
#include "GLExtensions.h"

#include <functional>


namespace LLGL
{


/* --- Internal functions --- */

template <typename T> bool LoadGLProc(T& procAddr, const char* procName)
{
    /*
    Load OpenGL procedure address
    -> Make an exception with platform dependent code here, because we use a template function.
    */
    #if defined(_WIN32)
    procAddr = reinterpret_cast<T>(wglGetProcAddress(procName));
    #elif defined(FORK_POSIX_PLATFORM)
    procAddr = reinterpret_cast<T>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(procName)));
    #else
    IO::Log::Error("OS not supported for loading OpenGL extensions");
    return false;
    #endif
    
    /* Check for errors */
    if (!procAddr)
    {
        IO::Log::Error("Loading OpenGL procedure \"" + ToStr(procName) + "\" failed");
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

/* --- Common extension loading functions --- */

ExtMapType QueryExtensions(bool useGLCoreProfile)
{
    ExtMapType extMap;

    const char* extString = nullptr;
    
    /* Filter standard GL extensions */
    if (useGLCoreProfile)
    {
        #if defined(GL_VERSION_3_0) && !defined(GL_GLEXT_PROTOTYPES)
        
        if (glGetStringi || LoadGLProc(glGetStringi, "glGetStringi"))
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

    #if defined(FORK_WINDOWS_PLATFORM) && defined(WGL_ARB_extensions_string)

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

void LoadAllExtensions(ExtMapType& extMap)
{
    /* Only load GL extensions once */
    static bool extAlreadyLoaded;

    if (extAlreadyLoaded)
        return;

    /* Internal extension loading lambda function */
    auto LoadExtension = [&](const std::string& extName, const std::function<bool(void)>& extLoadingProc) -> void
    {
        auto it = extMap.find(extName);
        if (it != extMap.end() && !extLoadingProc())
        {
            IO::Log::Error("Loading OpenGL extension \"" + extName + "\" failed");
            it->second = false;
        }
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
    LoadExtension( "GL_ARB_sampler_objects",              LoadSamplerProcs               );

    /* Load misc extensions */
    LoadExtension( "GL_ARB_viewport_array",               LoadViewportArrayProcs         );
    LoadExtension( "GL_ARB_draw_buffers_blend",           LoadDrawBuffersBlendProcs      );
    LoadExtension( "GL_ARB_occlusion_query",              LoadQueryObjectProcs           );
    LoadExtension( "GL_ARB_multi_bind",                   LoadMultiBindProcs             );
    LoadExtension( "GL_EXT_stencil_two_side",             LoadStencilSeparateProcs       ); // <--- correct extension ???
    LoadExtension( "GL_KHR_debug",                        LoadDebugProcs                 );

    extAlreadyLoaded = true;
}

#define LoadVerbatimGLProc(n) LoadGLProc(n, #n)

/* --- Common GL extensions --- */

bool LoadSwapIntervalProcs()
{
    #if defined(FORK_WINDOWS_PLATFORM)
    return LoadVerbatimGLProc(wglSwapIntervalEXT);
    #elif defined(FORK_POSIX_PLATFORM)
    return LoadVerbatimGLProc(glXSwapIntervalSGI);
    #else
    return false;
    #endif
}

bool LoadPixelFormatProcs()
{
    #if defined(FORK_WINDOWS_PLATFORM)
    return LoadVerbatimGLProc(wglChoosePixelFormatARB);
    #else
    return false;
    #endif
}

bool LoadCreateContextProcs()
{
    #if defined(FORK_WINDOWS_PLATFORM)
    return LoadVerbatimGLProc(wglCreateContextAttribsARB);
    #else
    return false;
    #endif
}

/* --- Hardware buffer extensions --- */

bool LoadVBOProcs()
{
    return
        LoadVerbatimGLProc( glGenBuffers    ) &&
        LoadVerbatimGLProc( glDeleteBuffers ) &&
        LoadVerbatimGLProc( glBindBuffer    ) &&
        LoadVerbatimGLProc( glBufferData    ) &&
        LoadVerbatimGLProc( glBufferSubData ) &&
        LoadVerbatimGLProc( glMapBuffer     ) &&
        LoadVerbatimGLProc( glUnmapBuffer   );
}

bool LoadVAOProcs()
{
    return
        LoadVerbatimGLProc( glGenVertexArrays    ) &&
        LoadVerbatimGLProc( glDeleteVertexArrays ) &&
        LoadVerbatimGLProc( glBindVertexArray    );
}

bool LoadFBOProcs()
{
    return
        LoadVerbatimGLProc( glGenRenderbuffers                    ) &&
        LoadVerbatimGLProc( glDeleteRenderbuffers                 ) &&
        LoadVerbatimGLProc( glBindRenderbuffer                    ) &&
        LoadVerbatimGLProc( glRenderbufferStorage                 ) &&
        LoadVerbatimGLProc( glRenderbufferStorageMultisample      ) &&

        LoadVerbatimGLProc( glGenFramebuffers                     ) &&
        LoadVerbatimGLProc( glDeleteFramebuffers                  ) &&
        LoadVerbatimGLProc( glBindFramebuffer                     ) &&
        LoadVerbatimGLProc( glCheckFramebufferStatus              ) &&

        LoadVerbatimGLProc( glFramebufferTexture                  ) &&
        LoadVerbatimGLProc( glFramebufferTexture1D                ) &&
        LoadVerbatimGLProc( glFramebufferTexture2D                ) &&
        LoadVerbatimGLProc( glFramebufferTexture3D                ) &&
        LoadVerbatimGLProc( glFramebufferTextureLayer             ) &&
        LoadVerbatimGLProc( glFramebufferRenderbuffer             ) &&
        LoadVerbatimGLProc( glGetFramebufferAttachmentParameteriv ) &&
        LoadVerbatimGLProc( glBlitFramebuffer                     ) &&

        LoadVerbatimGLProc( glGenerateMipmap                      );
}

bool LoadUBOProcs()
{
    return
        LoadVerbatimGLProc( glGetUniformBlockIndex      ) &&
        LoadVerbatimGLProc( glGetActiveUniformBlockiv   ) &&
        LoadVerbatimGLProc( glGetActiveUniformBlockName ) &&
        LoadVerbatimGLProc( glUniformBlockBinding       ) &&
        LoadVerbatimGLProc( glBindBufferBase            );
}

bool LoadSSBOProcs()
{
    return LoadVerbatimGLProc(glShaderStorageBlockBinding);
}

/* --- Drawing extensions --- */

bool LoadDrawBuffersProcs()
{
    return LoadVerbatimGLProc(glDrawBuffers);
}

bool LoadInstancedProcs()
{
    return
        LoadVerbatimGLProc( glDrawArraysInstanced   ) &&
        LoadVerbatimGLProc( glDrawElementsInstanced );
}

bool LoadInstancedOffsetProcs()
{
    return
        LoadVerbatimGLProc( glDrawArraysInstancedBaseInstance             ) &&
        LoadVerbatimGLProc( glDrawElementsInstancedBaseInstance           ) &&
        LoadVerbatimGLProc( glDrawElementsInstancedBaseVertexBaseInstance );
}

bool LoadBaseVertexProcs()
{
    return
        LoadVerbatimGLProc( glDrawElementsBaseVertex          ) &&
        LoadVerbatimGLProc( glDrawElementsInstancedBaseVertex );
}

/* --- Shader extensions --- */

bool LoadShaderProcs()
{
    return
        LoadVerbatimGLProc( glCreateShader      ) &&
        LoadVerbatimGLProc( glShaderSource      ) &&
        LoadVerbatimGLProc( glCompileShader     ) &&
        LoadVerbatimGLProc( glGetShaderiv       ) &&
        LoadVerbatimGLProc( glGetShaderInfoLog  ) &&
        LoadVerbatimGLProc( glDeleteShader      ) &&

        LoadVerbatimGLProc( glCreateProgram     ) &&
        LoadVerbatimGLProc( glDeleteProgram     ) &&
        LoadVerbatimGLProc( glAttachShader      ) &&
        LoadVerbatimGLProc( glDetachShader      ) &&
        LoadVerbatimGLProc( glLinkProgram       ) &&
        LoadVerbatimGLProc( glValidateProgram   ) &&
        LoadVerbatimGLProc( glGetProgramiv      ) &&
        LoadVerbatimGLProc( glGetProgramInfoLog ) &&
        LoadVerbatimGLProc( glUseProgram        ) &&

        LoadVerbatimGLProc( glGetActiveAttrib   ) &&
        LoadVerbatimGLProc( glGetAttribLocation );
}

bool LoadVertexAttribProcs()
{
    return
        LoadVerbatimGLProc( glEnableVertexAttribArray  ) &&
        LoadVerbatimGLProc( glDisableVertexAttribArray ) &&
        LoadVerbatimGLProc( glVertexAttribPointer      ) &&
        LoadVerbatimGLProc( glBindAttribLocation       );
}

bool LoadTessellationShaderProcs()
{
    return
        LoadVerbatimGLProc( glPatchParameteri  ) &&
        LoadVerbatimGLProc( glPatchParameterfv );
}

bool LoadComputeShaderProcs()
{
    return
        LoadVerbatimGLProc( glDispatchCompute         ) &&
        LoadVerbatimGLProc( glDispatchComputeIndirect );
}

bool LoadProgramBinaryProcs()
{
    return
        LoadVerbatimGLProc( glGetProgramBinary  ) &&
        LoadVerbatimGLProc( glProgramBinary     ) &&
        LoadVerbatimGLProc( glProgramParameteri );
}

bool LoadProgramInterfaceQueryProcs()
{
    return
        LoadVerbatimGLProc( glGetProgramInterfaceiv           ) &&
        LoadVerbatimGLProc( glGetProgramResourceIndex         ) &&
        LoadVerbatimGLProc( glGetProgramResourceName          ) &&
        LoadVerbatimGLProc( glGetProgramResourceiv            ) &&
        LoadVerbatimGLProc( glGetProgramResourceLocation      ) &&
        LoadVerbatimGLProc( glGetProgramResourceLocationIndex );
}

/* --- Texture extensions --- */

bool LoadMultiTextureProcs()
{
    return LoadVerbatimGLProc(glActiveTexture);
}

bool Load3DTextureProcs()
{
    return
        LoadVerbatimGLProc( glTexImage3D    ) &&
        LoadVerbatimGLProc( glTexSubImage3D );
}

bool LoadClearTextureProcs()
{
    return
        LoadVerbatimGLProc( glClearTexImage    ) &&
        LoadVerbatimGLProc( glClearTexSubImage );
}

bool LoadSamplerProcs()
{
    return
        LoadVerbatimGLProc( glGenSamplers        ) &&
        LoadVerbatimGLProc( glDeleteSamplers     ) &&
        LoadVerbatimGLProc( glBindSampler        ) &&
        LoadVerbatimGLProc( glSamplerParameteri  ) &&
        LoadVerbatimGLProc( glSamplerParameterf  ) &&
        LoadVerbatimGLProc( glSamplerParameteriv ) &&
        LoadVerbatimGLProc( glSamplerParameterfv );
}

/* --- Other extensions --- */

bool LoadQueryObjectProcs()
{
    return
        LoadVerbatimGLProc( glGenQueries        ) &&
        LoadVerbatimGLProc( glDeleteQueries     ) &&
        LoadVerbatimGLProc( glBeginQuery        ) &&
        LoadVerbatimGLProc( glEndQuery          ) &&
        LoadVerbatimGLProc( glGetQueryObjectiv  ) &&
        LoadVerbatimGLProc( glGetQueryObjectuiv );
}

bool LoadViewportArrayProcs()
{
    return
        LoadVerbatimGLProc( glViewportArrayv   ) &&
        LoadVerbatimGLProc( glScissorArrayv    ) &&
        LoadVerbatimGLProc( glDepthRangeArrayv );
}

bool LoadDrawBuffersBlendProcs()
{
    return
        LoadVerbatimGLProc( glBlendFuncSeparate  ) &&
        LoadVerbatimGLProc( glBlendFuncSeparatei );
}

bool LoadMultiBindProcs()
{
    return
        LoadVerbatimGLProc( glBindBuffersBase   ) &&
        LoadVerbatimGLProc( glBindBuffersRange  ) &&
        LoadVerbatimGLProc( glBindTextures      ) &&
        LoadVerbatimGLProc( glBindSamplers      ) &&
        LoadVerbatimGLProc( glBindImageTextures ) &&
        LoadVerbatimGLProc( glBindVertexBuffers );
}

bool LoadStencilSeparateProcs()
{
    return
        LoadVerbatimGLProc( glStencilFuncSeparate ) &&
        LoadVerbatimGLProc( glStencilMaskSeparate ) &&
        LoadVerbatimGLProc( glStencilOpSeparate   );
}

bool LoadDebugProcs()
{
    return LoadVerbatimGLProc(glDebugMessageCallback);
}

#undef LoadVerbatimGLProc


} // /namespace LLGL

#endif



// ================================================================================
