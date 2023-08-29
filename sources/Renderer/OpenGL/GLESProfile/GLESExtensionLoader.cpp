/*
 * GLESExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Ext/GLExtensionLoader.h"
#include "GLESExtensions.h"
#include "GLESExtensionsProxy.h"
#include "OpenGLES.h"
#if defined(LLGL_OS_IOS)
//#   import <OpenGLES/EAGL.h>
#else
#   include <EGL/egl.h>
#endif
#include <LLGL/Utils/ForRange.h>
#include <functional>


namespace LLGL
{


/* --- Internal functions --- */

#ifndef LLGL_OS_IOS

template <typename T>
bool LoadGLProc(T& procAddr, const char* procName)
{
    /* Load OpenGLES procedure address with EGL */
    procAddr = eglGetProcAddress(procName);
    return (procAddr != nullptr);
}


using LoadGLExtensionProc = std::function<bool(const char* extName, bool abortOnFailure, bool usePlaceholder)>;

#define DECL_LOADGLEXT_PROC(EXTNAME) \
    Load_ ## EXTNAME(const char* extName, bool abortOnFailure, bool usePlaceholder)

#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#define LOAD_GLPROC(NAME)                                                               \
    if (usePlaceholder)                                                                 \
    {                                                                                   \
        NAME = Proxy_##NAME;                                                            \
    }                                                                                   \
    else if (!LoadGLProc(NAME, #NAME))                                                  \
    {                                                                                   \
        if (abortOnFailure)                                                             \
            LLGL_TRAP("failed to load OpenGLES procedure: %s ( %s )", #NAME, extName);  \
        return false;                                                                   \
    }

/* --- Common GLES extensions --- */

/*static bool DECL_LOADGLEXT_PROC(GL_OES_tessellation_shader)
{
    LOAD_GLPROC( glPatchParameteriOES );
    return true;
}

static bool DECL_LOADGLEXT_PROC(GL_ARB_compute_shader)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}*/

#undef DECL_LOADGLEXT_PROC
#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC


/* --- Common extension loading functions --- */

static GLExtensionList QuerySupportedOpenGLExtensions(bool coreProfile)
{
    GLExtensionList extensions;

    /* Get number of extensions */
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for_range(i, numExtensions)
    {
        /* Get current extension string */
        if (auto extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)))
            extensions[extString] = false;
    }

    return extensions;
}

#endif // /LLGL_OS_IOS

// Global member to store if the extension have already been loaded
static bool g_OpenGLExtensionsLoaded = false;

bool LoadSupportedOpenGLExtensions(bool isCoreProfile, bool abortOnFailure)
{
    /* Only load GL extensions once */
    if (g_OpenGLExtensionsLoaded)
        return true;

    #ifdef LLGL_OS_IOS

    //TODO: only enable extensions

    #else

    GLExtensionMap extensions = QuerySupportedOpenGLExtensions(isCoreProfile);

    auto LoadExtension = [&extensions, abortOnFailure](const char* extName, const LoadGLExtensionProc& extLoadingProc, GLExt extensionID) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extensions.find(extName);
        if (it != extensions.end())
        {
            if (extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ false))
            {
                /* Enable extension in registry */
                RegisterExtension(extensionID);
                it->second = true;
            }
            else
            {
                /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
                extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ true);
            }
        }
        else
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(extName, abortOnFailure, /*usePlaceholder:*/ true);
        }
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" #NAME, Load_GL_##NAME, GLExt::NAME)

    /* Load hardware buffer extensions */
    //LOAD_GLEXT( OES_tessellation_shader );
    //LOAD_GLEXT( ARB_compute_shader      );

    #undef LOAD_GLEXT

    #endif // /LLGL_OS_IOS

    g_OpenGLExtensionsLoaded = true;

    return true;
}

bool AreOpenGLExtensionsLoaded()
{
    return g_OpenGLExtensionsLoaded;
}


} // /namespace LLGL



// ================================================================================
