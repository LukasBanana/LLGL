/*
 * GLSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SWAP_CHAIN_H
#define LLGL_GL_SWAP_CHAIN_H


#include <LLGL/Window.h>
#include <LLGL/SwapChain.h>
#include <LLGL/RendererConfiguration.h>
#include "OpenGL.h"
#include "RenderState/GLStateManager.h"
#include "Platform/GLContext.h"
#include "Platform/GLSwapChainContext.h"
#include <memory>


namespace LLGL
{


struct NativeHandle;
class GLRenderTarget;
class GLRenderSystem;
class GLContextManager;

class GLSwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        GLSwapChain(
            GLRenderSystem&                 renderSystem,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            GLContextManager&               contextMngr
        );

        // Makes the swap-chain's GL context current and updates the renger-target height in the linked GL state manager.
        static bool MakeCurrent(GLSwapChain* swapChain);

        // Returns the state manager of the swap chain's GL context.
        inline GLStateManager& GetStateManager()
        {
            return context_->GetStateManager();
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetSwapInterval(int swapInterval);

        #ifdef __linux__
        void ChooseGLXVisualAndGetX11WindowContext(GLPixelFormat& pixelFormat, NativeHandle& windowContext);
        #endif

        void BuildAndSetDefaultSurfaceTitle(const RendererInfo& info);

    private:

        std::shared_ptr<GLContext>          context_;
        std::unique_ptr<GLSwapChainContext> swapChainContext_;
        GLint                               framebufferHeight_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
