/*
 * GLSwapChain.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSwapChain.h"


namespace LLGL
{


GLSwapChain::GLSwapChain(
    const SwapChainDescriptor&          desc,
    const RendererConfigurationOpenGL&  config,
    const std::shared_ptr<Surface>&     surface,
    GLSwapChain*                        sharedSwapChain)
:
    SwapChain      { desc                                       },
    contextHeight_ { static_cast<GLint>(desc.resolution.height) }
{
    GLContext* sharedGLContext = (sharedSwapChain != nullptr ? sharedSwapChain->context_.get() : nullptr);

    #ifdef LLGL_OS_LINUX

    auto finalSwapChainDesc = desc;

    /* Setup surface for the swap-chain and pass native context handle */
    NativeContextHandle windowContext;
    ChooseGLXVisualAndGetX11WindowContext(desc, finalSwapChainDesc.samples, windowContext);
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, &windowContext);

    /* Create platform dependent OpenGL context with modified descriptor in case multisamples have changed */
    context_ = GLContext::Create(finalSwapChainDesc, config, GetSurface(), sharedGLContext);

    #else

    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    /* Create platform dependent OpenGL context */
    context_ = GLContext::Create(desc, config, GetSurface(), sharedGLContext);

    #endif

    /* Get state manager and notify about the current render context */
    stateMngr_ = context_->GetStateManager();
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);

    /* Initialize render states for the first time */
    if (!sharedSwapChain)
        InitRenderStates();
}

void GLSwapChain::Present()
{
    context_->SwapBuffers();
}

std::uint32_t GLSwapChain::GetSamples() const
{
    return context_->GetSamples();
}

Format GLSwapChain::GetColorFormat() const
{
    return context_->GetColorFormat();
}

Format GLSwapChain::GetDepthStencilFormat() const
{
    return context_->GetDepthStencilFormat();
}

const RenderPass* GLSwapChain::GetRenderPass() const
{
    return nullptr; // dummy
}

bool GLSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return SetSwapInterval(static_cast<int>(vsyncInterval));
}

bool GLSwapChain::GLMakeCurrent(GLSwapChain* swapChain)
{
    if (swapChain)
    {
        /* Make OpenGL context of the specified render contex current and notify the state manager */
        auto result = GLContext::MakeCurrent(swapChain->context_.get());
        GLStateManager::Get().NotifyRenderTargetHeight(swapChain->contextHeight_);
        return result;
    }
    else
        return GLContext::MakeCurrent(nullptr);
}


/*
 * ======= Private: =======
 */

bool GLSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    /* Notify GL context of a resize */
    context_->Resize(resolution);

    /* Update context height */
    contextHeight_ = static_cast<GLint>(resolution.height);
    stateMngr_->NotifyRenderTargetHeight(contextHeight_);

    return true;
}

bool GLSwapChain::SetSwapInterval(int swapInterval)
{
    return context_->SetSwapInterval(swapInterval);
}

void GLSwapChain::InitRenderStates()
{
    /* Initialize state manager */
    stateMngr_->Reset();

    /* D3D11, Vulkan, and Metal always use a fixed restart index for strip topologies */
    #ifdef LLGL_PRIMITIVE_RESTART_FIXED_INDEX
    stateMngr_->Enable(GLState::PRIMITIVE_RESTART_FIXED_INDEX);
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
