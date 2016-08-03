/*
 * GLModuleInterface.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLExtensionLoader.h"
#include "GLExtensions.h"
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

/* --- Common extension loading functions --- */

OpenGLExtensionMap QueryExtensions(bool coreProfile)
{
    OpenGLExtensionMap extMap;

    const char* extString = nullptr;
    
    /* Filter standard GL extensions */
    if (coreProfile)
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
            Log::StdErr() << "failed to load OpenGL extension: " << extName << std::endl;
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

#define LOAD_VERBATIM_GLPROC(NAME) LoadGLProc(NAME, #NAME)

/* --- Common GL extensions --- */

bool LoadSwapIntervalProcs()
{
    #if defined(_WIN32)
    return LOAD_VERBATIM_GLPROC(wglSwapIntervalEXT);
    #elif defined(__linux__)
    return LOAD_VERBATIM_GLPROC(glXSwapIntervalSGI);
    #else
    return false;
    #endif
}

bool LoadPixelFormatProcs()
{
    #if defined(_WIN32)
    return LOAD_VERBATIM_GLPROC(wglChoosePixelFormatARB);
    #else
    return false;
    #endif
}

bool LoadCreateContextProcs()
{
    #if defined(_WIN32)
    return LOAD_VERBATIM_GLPROC(wglCreateContextAttribsARB);
    #else
    return false;
    #endif
}

/* --- Hardware buffer extensions --- */

bool LoadVBOProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGenBuffers    ) &&
        LOAD_VERBATIM_GLPROC( glDeleteBuffers ) &&
        LOAD_VERBATIM_GLPROC( glBindBuffer    ) &&
        LOAD_VERBATIM_GLPROC( glBufferData    ) &&
        LOAD_VERBATIM_GLPROC( glBufferSubData ) &&
        LOAD_VERBATIM_GLPROC( glMapBuffer     ) &&
        LOAD_VERBATIM_GLPROC( glUnmapBuffer   );
}

bool LoadVAOProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGenVertexArrays    ) &&
        LOAD_VERBATIM_GLPROC( glDeleteVertexArrays ) &&
        LOAD_VERBATIM_GLPROC( glBindVertexArray    );
}

bool LoadFBOProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGenRenderbuffers                    ) &&
        LOAD_VERBATIM_GLPROC( glDeleteRenderbuffers                 ) &&
        LOAD_VERBATIM_GLPROC( glBindRenderbuffer                    ) &&
        LOAD_VERBATIM_GLPROC( glRenderbufferStorage                 ) &&
        LOAD_VERBATIM_GLPROC( glRenderbufferStorageMultisample      ) &&

        LOAD_VERBATIM_GLPROC( glGenFramebuffers                     ) &&
        LOAD_VERBATIM_GLPROC( glDeleteFramebuffers                  ) &&
        LOAD_VERBATIM_GLPROC( glBindFramebuffer                     ) &&
        LOAD_VERBATIM_GLPROC( glCheckFramebufferStatus              ) &&

        LOAD_VERBATIM_GLPROC( glFramebufferTexture                  ) &&
        LOAD_VERBATIM_GLPROC( glFramebufferTexture1D                ) &&
        LOAD_VERBATIM_GLPROC( glFramebufferTexture2D                ) &&
        LOAD_VERBATIM_GLPROC( glFramebufferTexture3D                ) &&
        LOAD_VERBATIM_GLPROC( glFramebufferTextureLayer             ) &&
        LOAD_VERBATIM_GLPROC( glFramebufferRenderbuffer             ) &&
        LOAD_VERBATIM_GLPROC( glGetFramebufferAttachmentParameteriv ) &&
        LOAD_VERBATIM_GLPROC( glBlitFramebuffer                     ) &&

        LOAD_VERBATIM_GLPROC( glGenerateMipmap                      );
}

bool LoadUBOProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGetUniformBlockIndex      ) &&
        LOAD_VERBATIM_GLPROC( glGetActiveUniformBlockiv   ) &&
        LOAD_VERBATIM_GLPROC( glGetActiveUniformBlockName ) &&
        LOAD_VERBATIM_GLPROC( glUniformBlockBinding       ) &&
        LOAD_VERBATIM_GLPROC( glBindBufferBase            );
}

bool LoadSSBOProcs()
{
    return LOAD_VERBATIM_GLPROC(glShaderStorageBlockBinding);
}

/* --- Drawing extensions --- */

bool LoadDrawBuffersProcs()
{
    return LOAD_VERBATIM_GLPROC(glDrawBuffers);
}

bool LoadInstancedProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glDrawArraysInstanced   ) &&
        LOAD_VERBATIM_GLPROC( glDrawElementsInstanced );
}

bool LoadInstancedOffsetProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glDrawArraysInstancedBaseInstance             ) &&
        LOAD_VERBATIM_GLPROC( glDrawElementsInstancedBaseInstance           ) &&
        LOAD_VERBATIM_GLPROC( glDrawElementsInstancedBaseVertexBaseInstance );
}

bool LoadBaseVertexProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glDrawElementsBaseVertex          ) &&
        LOAD_VERBATIM_GLPROC( glDrawElementsInstancedBaseVertex );
}

/* --- Shader extensions --- */

bool LoadShaderProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glCreateShader      ) &&
        LOAD_VERBATIM_GLPROC( glShaderSource      ) &&
        LOAD_VERBATIM_GLPROC( glCompileShader     ) &&
        LOAD_VERBATIM_GLPROC( glGetShaderiv       ) &&
        LOAD_VERBATIM_GLPROC( glGetShaderInfoLog  ) &&
        LOAD_VERBATIM_GLPROC( glDeleteShader      ) &&

        LOAD_VERBATIM_GLPROC( glCreateProgram     ) &&
        LOAD_VERBATIM_GLPROC( glDeleteProgram     ) &&
        LOAD_VERBATIM_GLPROC( glAttachShader      ) &&
        LOAD_VERBATIM_GLPROC( glDetachShader      ) &&
        LOAD_VERBATIM_GLPROC( glLinkProgram       ) &&
        LOAD_VERBATIM_GLPROC( glValidateProgram   ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramiv      ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramInfoLog ) &&
        LOAD_VERBATIM_GLPROC( glUseProgram        ) &&

        LOAD_VERBATIM_GLPROC( glGetActiveAttrib   ) &&
        LOAD_VERBATIM_GLPROC( glGetAttribLocation );
}

bool LoadVertexAttribProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glEnableVertexAttribArray  ) &&
        LOAD_VERBATIM_GLPROC( glDisableVertexAttribArray ) &&
        LOAD_VERBATIM_GLPROC( glVertexAttribPointer      ) &&
        LOAD_VERBATIM_GLPROC( glBindAttribLocation       );
}

bool LoadTessellationShaderProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glPatchParameteri  ) &&
        LOAD_VERBATIM_GLPROC( glPatchParameterfv );
}

bool LoadComputeShaderProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glDispatchCompute         ) &&
        LOAD_VERBATIM_GLPROC( glDispatchComputeIndirect );
}

bool LoadProgramBinaryProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGetProgramBinary  ) &&
        LOAD_VERBATIM_GLPROC( glProgramBinary     ) &&
        LOAD_VERBATIM_GLPROC( glProgramParameteri );
}

bool LoadProgramInterfaceQueryProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGetProgramInterfaceiv           ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramResourceIndex         ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramResourceName          ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramResourceiv            ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramResourceLocation      ) &&
        LOAD_VERBATIM_GLPROC( glGetProgramResourceLocationIndex );
}

/* --- Texture extensions --- */

bool LoadMultiTextureProcs()
{
    return LOAD_VERBATIM_GLPROC(glActiveTexture);
}

bool Load3DTextureProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glTexImage3D    ) &&
        LOAD_VERBATIM_GLPROC( glTexSubImage3D );
}

bool LoadClearTextureProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glClearTexImage    ) &&
        LOAD_VERBATIM_GLPROC( glClearTexSubImage );
}

bool LoadSamplerProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGenSamplers        ) &&
        LOAD_VERBATIM_GLPROC( glDeleteSamplers     ) &&
        LOAD_VERBATIM_GLPROC( glBindSampler        ) &&
        LOAD_VERBATIM_GLPROC( glSamplerParameteri  ) &&
        LOAD_VERBATIM_GLPROC( glSamplerParameterf  ) &&
        LOAD_VERBATIM_GLPROC( glSamplerParameteriv ) &&
        LOAD_VERBATIM_GLPROC( glSamplerParameterfv );
}

/* --- Other extensions --- */

bool LoadQueryObjectProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glGenQueries        ) &&
        LOAD_VERBATIM_GLPROC( glDeleteQueries     ) &&
        LOAD_VERBATIM_GLPROC( glBeginQuery        ) &&
        LOAD_VERBATIM_GLPROC( glEndQuery          ) &&
        LOAD_VERBATIM_GLPROC( glGetQueryObjectiv  ) &&
        LOAD_VERBATIM_GLPROC( glGetQueryObjectuiv );
}

bool LoadViewportArrayProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glViewportArrayv   ) &&
        LOAD_VERBATIM_GLPROC( glScissorArrayv    ) &&
        LOAD_VERBATIM_GLPROC( glDepthRangeArrayv );
}

bool LoadDrawBuffersBlendProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glBlendFuncSeparate  ) &&
        LOAD_VERBATIM_GLPROC( glBlendFuncSeparatei );
}

bool LoadMultiBindProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glBindBuffersBase   ) &&
        LOAD_VERBATIM_GLPROC( glBindBuffersRange  ) &&
        LOAD_VERBATIM_GLPROC( glBindTextures      ) &&
        LOAD_VERBATIM_GLPROC( glBindSamplers      ) &&
        LOAD_VERBATIM_GLPROC( glBindImageTextures ) &&
        LOAD_VERBATIM_GLPROC( glBindVertexBuffers );
}

bool LoadStencilSeparateProcs()
{
    return
        LOAD_VERBATIM_GLPROC( glStencilFuncSeparate ) &&
        LOAD_VERBATIM_GLPROC( glStencilMaskSeparate ) &&
        LOAD_VERBATIM_GLPROC( glStencilOpSeparate   );
}

bool LoadDebugProcs()
{
    return LOAD_VERBATIM_GLPROC(glDebugMessageCallback);
}

#undef LOAD_VERBATIM_GLPROC


} // /namespace LLGL



// ================================================================================
