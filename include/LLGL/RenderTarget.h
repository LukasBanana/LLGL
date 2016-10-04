/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_TARGET_H__
#define __LLGL_RENDER_TARGET_H__


#include "Export.h"
#include "TextureFlags.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


//! Render target attachment descriptor structure.
struct RenderTargetAttachmentDescriptor
{
    unsigned int    mipLevel    = 0;                    //!< MIP-map level.
    unsigned int    layer       = 0;                    //!< Array texture layer.
    AxisDirection   cubeFace    = AxisDirection::XPos;  //!< Cube texture face.
};

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

        /**
        \brief Attaches an internal depth buffer to this render target.
        \param[in] size Specifies the size of the depth buffer. This must be the same as for all other attachemnts.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthStencilBuffer
        */
        virtual void AttachDepthBuffer(const Gs::Vector2ui& size) = 0;

        /**
        \brief Attaches an internal stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        */
        virtual void AttachStencilBuffer(const Gs::Vector2ui& size) = 0;

        /**
        \brief Attaches an internal depth-stencil buffer to this render target.
        \remarks Only a single depth buffer, stencil buffer, or depth-stencil buffer can be attached.
        \see AttachDepthBuffer
        */
        virtual void AttachDepthStencilBuffer(const Gs::Vector2ui& size) = 0;

        /**
        \brief Attaches the specified texture to this render target.
        \param[in] attachmnetDesc Specifies the attachment descriptor.
        Unused members will be ignored, e.g. the 'layer' member is ignored when a non-array texture is passed.
        */
        virtual void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) = 0;

        //! Detaches all textures and depth-stencil buffers from this render target.
        virtual void DetachAll() = 0;

        /**
        \brief Returns the frame buffer resolution.
        \remarks This will be determined by the first texture attachment. Every further attachment must have the same size.
        */
        inline const Gs::Vector2ui& GetResolution() const
        {
            return resolution_;
        }

    protected:

        void ApplyResolution(const Gs::Vector2ui& resolution);
        void ApplyMipResolution(Texture& texture, unsigned int mipLevel);
        void ResetResolution();

    private:

        Gs::Vector2ui resolution_;

};


} // /namespace LLGL


#endif



// ================================================================================
