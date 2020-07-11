/*
 * GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContext.h"


namespace LLGL
{


GLRenderContext::GLRenderContext(
    RenderContextDescriptor             desc,
    const RendererConfigurationOpenGL&  config,
    const std::shared_ptr<Surface>&     surface,
    GLRenderContext*                    sharedRenderContext)
:
    RenderContext  { desc.videoMode, desc.vsync                           },
    contextHeight_ { static_cast<GLint>(desc.videoMode.resolution.height) }
{
    #ifdef LLGL_OS_LINUX

    /* Setup surface for the render context and pass native context handle */
    NativeContextHandle windowContext;
    GetNativeContextHandle(windowContext, desc.videoMode, desc.samples);
    SetOrCreateSurface(surface, desc.videoMode, &windowContext);

    #else

    /* Setup surface for the render context */
    SetOrCreateSurface(surface, desc.videoMode, nullptr);

    #endif

    /* Update video mode of descriptor after surface has been set or created */
    desc.videoMode = GetVideoMode();

    /* Create platform dependent OpenGL context */
    GLContext* sharedGLContext = (sharedRenderContext != nullptr ? sharedRenderContext->context_.get() : nullptr);
    context_ = GLContext::Create(desc, config, GetSurface(), sharedGLContext);

    /* Setup swap interval (for v-sync) */
    OnSetVsync(desc.vsync);

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

std::uint32_t GLRenderContext::GetSamples() const
{
    return context_->GetSamples();
}

Format GLRenderContext::GetColorFormat() const
{
    /* Return fixed value, not much of control for an OpenGL context */
    return Format::RGBA8UNorm;
}

Format GLRenderContext::GetDepthStencilFormat() const
{
    /* Return fixed value, not much of control for an OpenGL context */
    return Format::D24UNormS8UInt;
}

const RenderPass* GLRenderContext::GetRenderPass() const
{
    return nullptr; // dummy
}

bool GLRenderContext::GLMakeCurrent(GLRenderContext* renderContext)
{
    if (renderContext)
    {
        /* Make OpenGL context of the specified render contex current and notify the state manager */
        auto result = GLContext::MakeCurrent(renderContext->context_.get());
        GLStateManager::Get().NotifyRenderTargetHeight(renderContext->contextHeight_);
        return result;
    }
    else
        return GLContext::MakeCurrent(nullptr);
}


/*
 * ======= Private: =======
 */

bool GLRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    /* Update context height */
    contextHeight_ = static_cast<GLint>(videoModeDesc.resolution.height);
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);

    /* Notify GL context of a resize */
    context_->Resize(videoModeDesc.resolution);

    /* Switch fullscreen mode */
    if (!SetDisplayFullscreenMode(videoModeDesc))
        return false;

    return true;
}

bool GLRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    int swapInterval = (vsyncDesc.enabled ? static_cast<int>(vsyncDesc.interval) : 0);
    return context_->SetSwapInterval(swapInterval);
}

void GLRenderContext::InitRenderStates()
{
    /* Initialize state manager */
    stateMngr_->Reset();

    /* D3D11, Vulkan, and Metal always use a fixed restart index for strip topologies */
    #ifdef LLGL_PRIMITIVE_RESTART_FIXED_INDEX
    stateMngr_->Enable(GLState::PRIMITIVE_RESTART_FIXED_INDEX);
    #elif defined LLGL_PRIMITIVE_RESTART
    stateMngr_->Enable(GLState::PRIMITIVE_RESTART);
    #endif

    #ifdef LLGL_OPENGL
    /* D3D10+ has this per default */
    stateMngr_->Enable(GLState::TEXTURE_CUBE_MAP_SEAMLESS);
    #endif

    /* D3D10+ uses clock-wise vertex winding per default */
    stateMngr_->SetFrontFace(GL_CW);

    /*
    Set pixel storage to byte-alignment (default is word-alignment).
    This is required so that texture formats like RGB (which is not word-aligned) can be used.
    */
    stateMngr_->SetPixelStorePack(0, 0, 1);
    stateMngr_->SetPixelStoreUnpack(0, 0, 1);
}


} // /namespace LLGL



// ================================================================================
