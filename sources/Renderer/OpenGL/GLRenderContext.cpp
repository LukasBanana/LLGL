/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"


namespace LLGL
{


GLRenderContext::GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Surface>& surface, GLRenderContext* sharedRenderContext) :
    desc_          { desc                        },
    contextHeight_ { desc.videoMode.resolution.y }
{
    #ifdef __linux__

    /* Setup surface for the render context and pass native context handle */
    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext);
    SetOrCreateSurface(surface, desc.videoMode, &windowContext);

    #else

    /* Setup surface for the render context */
    SetOrCreateSurface(surface, desc.videoMode, nullptr);

    #endif

    /* Create platform dependent OpenGL context */
    context_ = GLContext::Create(desc_, GetSurface(), (sharedRenderContext != nullptr ? sharedRenderContext->context_.get() : nullptr));

    /* Setup swap interval (for v-sync) */
    UpdateSwapInterval();

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
        
        /* Notify GL context of a resize */
        context_->Resize(videoModeDesc.resolution);
    }
}

void GLRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        UpdateSwapInterval();
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

void GLRenderContext::UpdateSwapInterval()
{
    context_->SetSwapInterval(desc_.vsync.enabled ? static_cast<int>(desc_.vsync.interval) : 0);
}


} // /namespace LLGL



// ================================================================================
