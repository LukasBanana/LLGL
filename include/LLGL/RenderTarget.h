/*
 * RenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_TARGET_H
#define LLGL_RENDER_TARGET_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/Types.h>


namespace LLGL
{


class Texture;

/**
\brief Render target interface.
\remarks A render target in the broader sense is a composition of Texture objects which can be specified as the destination for drawing operations.
After a texture has been attached to a render target, its image content is undefined until something has been rendered into the render target.
The only interface that inherits from this interface is SwapChain, a special case of render targets used to present the result on the screen.
\see RenderSystem::CreateRenderTarget
\see CommandBuffer::BeginRenderPass
\see SwapChain
*/
class LLGL_EXPORT RenderTarget : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::RenderTarget );

    public:

        /**
        \brief Returns the render target resolution.
        \remarks This is either determined by the resolution specified in the render target descriptor or swap-chain descriptor.
        \see SwapChain::ResizeBuffers
        \see RenderTargetDescriptor::resolution
        \see SwapChainDescriptor::resolution
        */
        virtual Extent2D GetResolution() const = 0;

        /**
        \brief Returns the number of samples this render target was created with.
        \remarks If a certain number of samples is not supported by the rendering API, LLGL will reduce the amount of samples.
        This function returns the actual number of samples the hardware object was created with.
        \see RenderTargetDescriptor::samples
        \see SwapChainDescriptor::samples
        */
        virtual std::uint32_t GetSamples() const = 0;

        /**
        \brief Returns the number of color attachments of this render target. This can also be zero.
        \remarks For a swap-chain, this will always be 1.
        \see SwapChain::GetColorFormat
        */
        virtual std::uint32_t GetNumColorAttachments() const = 0;

        /**
        \brief Returns true if this render target has a depth or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        \see SwapChain::GetDepthStencilFormat
        */
        virtual bool HasDepthAttachment() const = 0;

        /**
        \brief Returns true if this render target has a stencil or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        \see SwapChain::GetDepthStencilFormat
        */
        virtual bool HasStencilAttachment() const = 0;

        /**
        \brief Returns the RenderPass object this render target is associated with, or null if render passes are optional for the the render system.
        \remarks This is either the RenderPass object that was passed to the descriptor when this render target was created,
        or it is the default RenderPass object that was created by the render target itself.
        \see RenderTargetDescriptor::renderPass
        */
        virtual const RenderPass* GetRenderPass() const = 0;

    protected:

        /**
        \brief Validates the specified attachment resolution for this render target.
        \remarks Throws an exception or aborts execution if the specified resolution is invalid.
        */
        void ValidateResolution(const Extent2D& attachmentResolution);

        /**
        \brief Applies the resolution of the texture MIP level.
        \see Texture::GetMipExtent
        \see ValidateResolution
        */
        void ValidateMipResolution(const Texture& texture, std::uint32_t mipLevel);

};


} // /namespace LLGL


#endif



// ================================================================================
