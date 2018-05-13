/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_TARGET_H
#define LLGL_RENDER_TARGET_H


#include "Export.h"
#include "RenderTargetFlags.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


class Texture;

/**
\brief Render target interface.
\remarks A render target in the broader sense is a composition of Texture objects
which can be specified as the destination for drawing operations.
After a texture has been attached to a render target, its image content is undefined
until something has been rendered into the render target.
*/
class LLGL_EXPORT RenderTarget
{

    public:

        virtual ~RenderTarget();

        #if 1 // DEPRECATED

        /**
        \brief Attaches an internal depth buffer to this render target.
        \param[in] size Specifies the size of the depth buffer. This must be the same as for all other attachemnts.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthStencilBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachDepthBuffer(const Gs::Vector2ui& size);

        /**
        \brief Attaches an internal stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachStencilBuffer(const Gs::Vector2ui& size);

        /**
        \brief Attaches an internal depth-stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        \deprecated Use RenderTargetDescriptor::attachments instead.
        */
        virtual void AttachDepthStencilBuffer(const Gs::Vector2ui& size);

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
        \todo Rename to "GetExtent".
        */
        inline const Extent2D& GetResolution() const
        {
            return resolution_;
        }

    protected:

        void ApplyResolution(const Extent2D& resolution);
        void ApplyMipResolution(Texture& texture, std::uint32_t mipLevel);
        void ResetResolution();

    private:

        Extent2D resolution_;

};


} // /namespace LLGL


#endif



// ================================================================================
