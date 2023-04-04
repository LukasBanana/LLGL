/*
 * GLSwapChain.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLSwapChain.h"
#include "../TextureUtils.h"
#include "Platform/GLContextManager.h"


namespace LLGL
{


GLSwapChain::GLSwapChain(
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface,
    GLContextManager&               contextMngr)
:
    SwapChain      { desc                                       },
    contextHeight_ { static_cast<GLint>(desc.resolution.height) }
{
    /* Set up pixel format for GL context */
    GLPixelFormat pixelFormat;
    pixelFormat.colorBits   = desc.colorBits;
    pixelFormat.depthBits   = desc.depthBits;
    pixelFormat.stencilBits = desc.stencilBits;
    pixelFormat.samples     = static_cast<int>(GetClampedSamples(desc.samples));

    #ifdef LLGL_OS_LINUX

    /* Set up surface for the swap-chain and pass native context handle */
    NativeContextHandle windowContext;
    ChooseGLXVisualAndGetX11WindowContext(pixelFormat, windowContext);
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, &windowContext);

    #else

    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    #endif

    /* Create platform dependent OpenGL context */
    context_ = contextMngr.AllocContext(&pixelFormat, &GetSurface());
    swapChainContext_ = GLSwapChainContext::Create(*context_, GetSurface());
    GLSwapChainContext::MakeCurrent(swapChainContext_.get());

    /* Get state manager and notify about the current render context */
    GetStateManager().NotifyRenderTargetHeight(contextHeight_);
}

void GLSwapChain::Present()
{
    swapChainContext_->SwapBuffers();
}

std::uint32_t GLSwapChain::GetSamples() const
{
    return static_cast<std::uint32_t>(context_->GetSamples());
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

bool GLSwapChain::MakeCurrent(GLSwapChain* swapChain)
{
    if (swapChain)
    {
        /* Make OpenGL context of the specified render contex current and notify the state manager */
        auto result = GLSwapChainContext::MakeCurrent(swapChain->swapChainContext_.get());
        GLStateManager::Get().NotifyRenderTargetHeight(swapChain->contextHeight_);
        return result;
    }
    else
        return GLSwapChainContext::MakeCurrent(nullptr);
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
    GetStateManager().NotifyRenderTargetHeight(contextHeight_);

    return true;
}

bool GLSwapChain::SetSwapInterval(int swapInterval)
{
    GLSwapChainContext::MakeCurrent(swapChainContext_.get());
    return GLContext::SetCurrentSwapInterval(swapInterval);
}


} // /namespace LLGL



// ================================================================================
