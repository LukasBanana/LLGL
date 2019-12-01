/*
 * GLESExtensionLoader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../Ext/GLExtensionLoader.h"
#include "GLESExtensions.h"
#include "GLESExtensionsProxy.h"
#include "OpenGLES.h"
#include <EGL/egl.h>
#include <LLGL/Log.h>
#include <functional>


namespace LLGL
{


/* --- Internal functions --- */

template <typename T>
bool LoadGLProc(T& procAddr, const char* procName)
{
    /* Load OpenGLES procedure address with EGL */
    procAddr = eglGetProcAddress(procName);

    /* Check for errors */
    if (!procAddr)
    {
        Log::PostReport(Log::ReportType::Error, "failed to load OpenGLES procedure: " + std::string(procName));
        return false;
    }

    return true;
}


#define LOAD_GLPROC_SIMPLE(NAME) \
    LoadGLProc(NAME, #NAME)

#ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS

#define LOAD_GLPROC(NAME)               \
    if (usePlaceholder)                 \
        NAME = Proxy_##NAME;            \
    else if (!LoadGLProc(NAME, #NAME))  \
        return false

#else

#define LOAD_GLPROC(NAME)           \
    if (!LoadGLProc(NAME, #NAME))   \
        return false

#endif // /LLGL_GL_ENABLE_EXT_PLACEHOLDERS

/* --- Common GLES extensions --- */

/*static bool Load_GL_OES_tessellation_shader(bool usePlaceholder)
{
    LOAD_GLPROC( glPatchParameteriOES );
    return true;
}

static bool Load_GL_ARB_compute_shader(bool usePlaceholder)
{
    LOAD_GLPROC( glDispatchCompute         );
    LOAD_GLPROC( glDispatchComputeIndirect );
    return true;
}*/

#undef LOAD_GLPROC_SIMPLE
#undef LOAD_GLPROC


/* --- Common extension loading functions --- */

GLExtensionList QueryExtensions(bool coreProfile)
{
    GLExtensionList extensions;

    /* Get number of extensions */
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int i = 0; i < numExtensions; ++i)
    {
        /* Get current extension string */
        if (auto extString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)))
            extensions[extString] = false;
    }

    return extensions;
}

// Global member to store if the extension have already been loaded
static bool g_extAlreadyLoaded = false;

void LoadAllExtensions(GLExtensionList& extensions, bool coreProfile)
{
    /* Only load GL extensions once */
    if (g_extAlreadyLoaded)
        return;

    auto LoadExtension = [&](const std::string& extName, const std::function<bool(bool)>& extLoadingProc, GLExt extensionID) -> void
    {
        /* Try to load OpenGL extension */
        auto it = extensions.find(extName);
        if (it != extensions.end())
        {
            if (extLoadingProc(false))
            {
                /* Enable extension in registry */
                RegisterExtension(extensionID);
                it->second = true;
            }
            else
            {
                /* Loading extension failed */
                Log::PostReport(Log::ReportType::Error, "failed to load OpenGLES extension: " + extName);
            }
        }
        #ifdef LLGL_GL_ENABLE_EXT_PLACEHOLDERS
        else
        {
            /* If failed, use dummy procedures to detect illegal use of OpenGL extension */
            extLoadingProc(true);
        }
        #endif
    };

    #define LOAD_GLEXT(NAME) \
        LoadExtension("GL_" + std::string(#NAME), Load_GL_##NAME, GLExt::NAME)

    /* Load hardware buffer extensions */
    //LOAD_GLEXT( OES_tessellation_shader );
    //LOAD_GLEXT( ARB_compute_shader      );

    #undef LOAD_GLEXT

    g_extAlreadyLoaded = true;
}

bool AreExtensionsLoaded()
{
    return g_extAlreadyLoaded;
}


} // /namespace LLGL



// ================================================================================
