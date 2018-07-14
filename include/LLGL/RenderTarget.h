/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
\remarks A render target in the broader sense is a composition of Texture objects
which can be specified as the destination for drawing operations.
After a texture has been attached to a render target, its image content is undefined
until something has been rendered into the render target.
\see RenderSystem::CreateRenderTarget
\see CommandBuffer::SetRenderTarget(RenderTarget&)
*/
class LLGL_EXPORT RenderTarget : public RenderSystemChild
{

    public:

        //! Returns the number of color attachments of this render target. This can also be zero.
        virtual std::uint32_t GetNumColorAttachments() const = 0;

        /**
        \brief Returns true if this render target has a depth or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        */
        virtual bool HasDepthAttachment() const = 0;

        /**
        \brief Returns true if this render target has a stencil or depth-stencil attachment.
        \remarks The return value depends on whether the rendering API supports depth-stencil formats where the depth and stencil components can be strictly separated.
        For example, if the render target was created with only a stencil attachment,
        LLGL may still create a depth-stencil buffer that results in both a depth and stencil component in one attachment.
        */
        virtual bool HasStencilAttachment() const = 0;

        /**
        \brief Returns the render target resolution.
        \remarks This will be determined by the first texture attachment. Every further attachment must have the same size.
        */
        inline const Extent2D& GetResolution() const
        {
            return resolution_;
        }

    protected:

        RenderTarget(const Extent2D& resolution);

        /**
        \brief Applies the specified resolution.
        \remarks This shoudl be called for each attachment.
        \throws std::invalid_argument If one of the resolution components is zero.
        \throws std::invalid_argument If the internal resolution has already been set and the input resolution is not equal to that previous resolution.
        */
        void ValidateResolution(const Extent2D& resolution);

        /**
        \breif Applies the resolution of the texture MIP level.
        \see Texture::QueryMipExtent
        \see ValidateResolution
        */
        void ValidateMipResolution(const Texture& texture, std::uint32_t mipLevel);

    private:

        Extent2D resolution_;

};


} // /namespace LLGL


#endif



// ================================================================================
