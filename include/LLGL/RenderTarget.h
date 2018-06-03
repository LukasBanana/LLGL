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

        #if 1 // DEPRECATED

        /**
        \brief Attaches an internal depth buffer to this render target.
        \param[in] size Specifies the size of the depth buffer. This must be the same as for all other attachemnts.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthStencilBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachDepthBuffer(const Extent2D& size);

        /**
        \brief Attaches an internal stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachStencilBuffer(const Extent2D& size);

        /**
        \brief Attaches an internal depth-stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachDepthStencilBuffer(const Extent2D& size);

        /**
        \brief Attaches the specified texture to this render target.
        \param[in] attachmnetDesc Specifies the attachment descriptor.
        Unused members will be ignored, e.g. the 'layer' member is ignored when a non-array texture is passed.
        \note A mixed attachment of multi-sample and non-multi-sample textures to a render-target is currently only supported with: Direct3D 11.
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc);

        /**
        \brief Detaches all textures and depth-stencil buffers from this render target.
        \deprecated Use RenderSystem::CreateRenderTarget instead, to create a new render target.
        */
        virtual void DetachAll();

        #endif // /DEPRECATED

        /**
        \brief Returns the render target resolution.
        \remarks This will be determined by the first texture attachment. Every further attachment must have the same size.
        */
        inline const Extent2D& GetResolution() const
        {
            return resolution_;
        }

    protected:

        /**
        \brief Applies the specified resolution.
        \remarks This shoudl be called for each attachment.
        \throws std::invalid_argument If one of the resolution components is zero.
        \throws std::invalid_argument If the internal resolution has already been set and the input resolution is not equal to that previous resolution.
        */
        void ApplyResolution(const Extent2D& resolution);

        /**
        \breif Applies the resolution of the texture MIP level.
        \see Texture::QueryMipLevelSize
        \see ApplyResolution
        */
        void ApplyMipResolution(Texture& texture, std::uint32_t mipLevel);

        //! Resets the render target resolution to (0, 0).
        void ResetResolution();

    private:

        Extent2D resolution_;

};


} // /namespace LLGL


#endif



// ================================================================================
