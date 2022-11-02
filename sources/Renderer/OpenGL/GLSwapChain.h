/*
 * GLSwapChain.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

#ifdef __linux__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


class GLRenderTarget;
class GLContextManager;

class GLSwapChain final : public SwapChain
{

    public:

        /* ----- Common ----- */

        GLSwapChain(
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            GLContextManager&               contextMngr
        );

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        /* ----- GLSwapChain specific functions ----- */

        // Makes the swap-chain's GL context current and updates the renger-target height in the linked GL state manager.
        static bool MakeCurrent(GLSwapChain* swapChain);

        // Returns the state manager of the swap chain's GL context.
        inline GLStateManager& GetStateManager()
        {
            return context_->GetStateManager();
        }

    private:

        struct RenderState
        {
            GLenum      drawMode            = GL_TRIANGLES;
            GLenum      indexBufferDataType = GL_UNSIGNED_INT;
            GLintptr    indexBufferStride   = 4;
        };

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetSwapInterval(int swapInterval);

        #ifdef __linux__
        void ChooseGLXVisualAndGetX11WindowContext(GLPixelFormat& pixelFormat, NativeContextHandle& windowContext);
        #endif

    private:

        std::shared_ptr<GLContext>          context_;
        std::unique_ptr<GLSwapChainContext> swapChainContext_;
        RenderState                         renderState_;
        GLint                               contextHeight_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
