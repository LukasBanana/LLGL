/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_TARGET_H
#define LLGL_RENDER_TARGET_H


#include "RenderSystemChild.h"
#include "RenderTargetFlags.h"
#include "Types.h"


namespace LLGL
{


class Texture;

/**
\brief Render target interface.
\remarks A render target in the broader sense is a composition of Texture objects which can be specified as the destination for drawing operations.
After a texture has been attached to a render target, its image content is undefined until something has been rendered into the render target.
The only interface that inherits from this interface is RenderContext, a special case of render targets used to present the result on the screen.
\see RenderSystem::CreateRenderTarget
\see CommandBuffer::SetRenderTarget(RenderTarget&)
\see RenderContext
*/
class LLGL_EXPORT RenderTarget : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::RenderTarget );

    public:

        /**
        \brief Returns true if this render target is an instance of RenderContext.
        \todo Replace by IsInstanceOf
        */
        bool IsRenderContext() const;

        /**
        \brief Returns the render target resolution.
        \remarks This is either determined by the resolution specified in the render target descriptor, or by the video mode of the render context.
        \see RenderContext::GetVideoMode
        \see RenderTargetDescriptor::resolution
        \see VideoModeDescriptor::resolution
        */
        virtual Extent2D GetResolution() const = 0;

        /**
        \brief Returns the number of samples this render target was created with.
        \remarks If a certain number of samples is not supported by the rendering API, LLGL will reduce the amount of samples.
        This function returns the actual number of samples the hardware object was created with.
        \see RenderTargetDescriptor::samples
        \see VideoModeDescriptor::samples
        */
        virtual std::uint32_t GetSamples() const = 0;

        /**
        \brief Returns the number of color attachments of this render target. This can also be zero.
        \remarks For a render context, this will always be 1.
        \see RenderContext::GetColorFormat
        */
        virtual std::uint32_t GetNumColorAttachments() const = 0;

        /**
        \brief Returns true if this render target has a depth or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        \see RenderContext::GetDepthStencilFormat
        */
        virtual bool HasDepthAttachment() const = 0;

        /**
        \brief Returns true if this render target has a stencil or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        \see RenderContext::GetDepthStencilFormat
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
        \brief Applies the specified resolution.
        \remarks This shoudl be called for each attachment.
        \throws std::invalid_argument If one of the resolution components is zero.
        \throws std::invalid_argument If the internal resolution has already been set and the input resolution is not equal to that previous resolution.
        */
        void ValidateResolution(const Extent2D& resolution);

        /**
        \brief Applies the resolution of the texture MIP level.
        \see Texture::GetMipExtent
        \see ValidateResolution
        */
        void ValidateMipResolution(const Texture& texture, std::uint32_t mipLevel);

    private:

        // Only RenderContext is supposed to override "OnIsRenderContext".
        friend class RenderContext;

        // Returns true if this render target is an instance of RenderContext. By default false.
        virtual bool OnIsRenderContext() const;

};


} // /namespace LLGL


#endif



// ================================================================================
