/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"
#include "GLRenderSystem.h"
#include "Ext/GLExtensions.h"
#include "../Assertion.h"

#ifndef __APPLE__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


GLRenderContext* GLRenderContext::activeRenderContext_ = nullptr;

GLRenderContext::GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext) :
    desc_           ( desc                        ),
    contextHeight_  ( desc.videoMode.resolution.y )
{
    #ifndef __APPLE__
    /* Setup window for the render context */
    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext);
    SetWindow(window, desc.videoMode, &windowContext);
    #endif

    /* Acquire state manager to efficiently change render states */
    AcquireStateManager(sharedRenderContext);

    /* Create platform dependent OpenGL context */
    CreateContext(sharedRenderContext);

    /* Initialize render states for the first time */
    if (!sharedRenderContext)
        InitRenderStates();
}

GLRenderContext::~GLRenderContext()
{
    /* Unset the current OpenGL render context if this is the active one */
    if (activeRenderContext_ == this)
        GLRenderContext::GLMakeCurrent(nullptr);

    /* Delete OpenGL render context */
    DeleteContext();
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
        SetupVsyncInterval();
    }
}


/*
 * ======= Private: =======
 */

void GLRenderContext::AcquireStateManager(GLRenderContext* sharedRenderContext)
{
    if (sharedRenderContext)
    {
        /* Share state manager with shared render context */
        stateMngr_ = sharedRenderContext->stateMngr_;
    }
    else
    {
        /* Create a new shared state manager */
        stateMngr_ = std::make_shared<GLStateManager>();
    }

    /* Notify state manager about the current render context */
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);
}

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
