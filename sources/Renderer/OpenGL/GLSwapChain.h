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
#include <memory>

#ifdef __linux__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


class GLRenderTarget;

class GLSwapChain final : public SwapChain
{

    public:

        /* ----- Common ----- */

        GLSwapChain(
            const SwapChainDescriptor&          desc,
            const RendererConfigurationOpenGL&  config,
            const std::shared_ptr<Surface>&     surface,
            GLSwapChain*                        sharedSwapChain
        );

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        /* ----- GLSwapChain specific functions ----- */

        static bool GLMakeCurrent(GLSwapChain* swapChain);

        inline const std::shared_ptr<GLStateManager>& GetStateManager() const
        {
            return stateMngr_;
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

        void InitRenderStates();

        #ifdef __linux__
        void ChooseGLXVisualAndGetX11WindowContext(
            const SwapChainDescriptor&  desc,
            std::uint32_t&              samples,
            NativeContextHandle&        windowContext
        );
        #endif

    private:

        std::unique_ptr<GLContext>      context_;

        std::shared_ptr<GLStateManager> stateMngr_;
        RenderState                     renderState_;

        GLint                           contextHeight_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
