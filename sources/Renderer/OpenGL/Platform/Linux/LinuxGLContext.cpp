/*
 * LinuxGLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxGLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Log.h>
#include <algorithm>


namespace LLGL
{


/*
 * GLContext class
 */

std::unique_ptr<GLContext> GLContext::Create(RenderContextDescriptor& desc, Surface& surface, GLContext* sharedContext)
{
    LinuxGLContext* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContext*, sharedContext) : nullptr);
    return MakeUnique<LinuxGLContext>(desc, surface, sharedContextGLX);
}


/*
 * LinuxGLContext class
 */

LinuxGLContext::LinuxGLContext(RenderContextDescriptor& desc, Surface& surface, LinuxGLContext* sharedContext) :
    GLContext { sharedContext }
{
    NativeHandle nativeHandle;
    surface.GetNativeHandle(&nativeHandle);
    
    if (sharedContext)
    {
        auto sharedContextGLX = LLGL_CAST(LinuxGLContext*, sharedContext);
        CreateContext(nativeHandle, sharedContextGLX);
    }
    else
        CreateContext(nativeHandle, nullptr);
}

LinuxGLContext::~LinuxGLContext()
{
    DeleteContext();
}

bool LinuxGLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "glXSwapIntervalSGI" to set v-sync interval */
    if (glXSwapIntervalSGI || LoadSwapIntervalProcs())
        return (glXSwapIntervalSGI(interval) == 0);
    else
        return false;
}

bool LinuxGLContext::SwapBuffers()
{
    glXSwapBuffers(display_, wnd_);
    return true;
}

void LinuxGLContext::Resize(const Size& resolution)
{
    //TODO...
}


/*
 * ======= Private: =======
 */

bool LinuxGLContext::Activate(bool activate)
{
    if (activate)
        return glXMakeCurrent(display_, wnd_, glc_);
    else
        return glXMakeCurrent(nullptr, 0, 0);
}

void LinuxGLContext::CreateContext(const NativeHandle& nativeHandle, LinuxGLContext* sharedContext)
{
    GLXContext glcShared = (sharedContext != nullptr ? sharedContext->glc_ : nullptr);
    
    /* Get X11 display, window, and visual information */
    display_    = nativeHandle.display;
    wnd_        = nativeHandle.window;
    visual_     = nativeHandle.visual;
    
    if (!display_ || !wnd_ || !visual_)
        throw std::invalid_argument("failed to create OpenGL context on X11 client, due to missing arguments");
    
    /* Create OpenGL context with X11 lib */
    glc_ = glXCreateContext(display_, visual_, glcShared, GL_TRUE);
    
    /* Make new OpenGL context current */
    if (glXMakeCurrent(display_, wnd_, glc_) != True)
        Log::StdErr() << "failed to make OpenGL render context current (glXMakeCurrent)" << std::endl;
}

void LinuxGLContext::DeleteContext()
{
    glXDestroyContext(display_, glc_);
}



} // /namespace LLGL



// ================================================================================
