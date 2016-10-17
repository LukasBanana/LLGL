/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"

#ifndef __APPLE__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


GLRenderContext::GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext) :
    desc_           ( desc                        ),
    contextHeight_  ( desc.videoMode.resolution.y )
{
    /* Setup window for the render context */
    #ifdef __APPLE__

    SetWindow(window, desc.videoMode, nullptr);

    #else

    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext);
    SetWindow(window, desc.videoMode, &windowContext);

    #endif

    /* Create platform dependent OpenGL context */
    context_ = GLContext::Create(desc_, GetWindow(), (sharedRenderContext != nullptr ? sharedRenderContext->context_.get() : nullptr));

    /* Get state manager and notify about the current render context */
    stateMngr_ = context_->GetStateManager();
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);

    /* Initialize render states for the first time */
    if (!sharedRenderContext)
        InitRenderStates();
}

void GLRenderContext::Present()
{
    context_->SwapBuffers();
}

/* ----- Configuration ----- */

void GLRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        /* Update context height */
        contextHeight_ = videoModeDesc.resolution.y;
        stateMngr_->NotifyRenderTargetHeight(contextHeight_);

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);
    }
}

void GLRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        context_->SetSwapInterval(desc_.vsync.enabled ? static_cast<int>(desc_.vsync.interval) : 0);
    }
}

bool GLRenderContext::GLMakeCurrent(GLRenderContext* renderContext)
{
    if (renderContext)
    {
        /* Make OpenGL context of the specified render contex current and notify the state manager */
        auto result = GLContext::MakeCurrent(renderContext->context_.get());
        GLStateManager::active->NotifyRenderTargetHeight(renderContext->contextHeight_);
        return result;
    }
    else
        return GLContext::MakeCurrent(nullptr);
}


/*
 * ======= Private: =======
 */

void GLRenderContext::InitRenderStates()
{
    /* Initialize state manager */
    stateMngr_->Reset();

    /* Setup default render states to be uniform between render systems */
    stateMngr_->Enable(GLState::TEXTURE_CUBE_MAP_SEAMLESS); // D3D10+ has this per default
    stateMngr_->SetFrontFace(GL_CW);                        // D3D10+ uses clock-wise vertex winding per default

    /*
    Set pixel storage to byte-alignment (default is word-alignment).
    This is required so that texture formats like RGB (which is not word-aligned) can be used.
    */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


} // /namespace LLGL



// ================================================================================
