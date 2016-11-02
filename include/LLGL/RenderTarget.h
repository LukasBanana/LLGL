/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_TARGET_H
#define LLGL_RENDER_TARGET_H


#include "Export.h"
#include "TextureFlags.h"
#include "GraphicsPipelineFlags.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


//! Render target attachment descriptor structure.
struct RenderTargetAttachmentDescriptor
{
    /**
    \brief Specifies the MIP-map level which is to be attached to a render target.
    \remarks This is only used for non-multi-sample textures.
    All multi-sample textures will always use the first MIP-map level
    (i.e. TextureType::Texture2DMS and TextureType::Texture2DMSArray).
    */
    unsigned int    mipLevel    = 0;

    /**
    \brief Array texture layer.
    \remarks This is only used for array textures (i.e. TextureType::Texture1DArray,
    TextureType::Texture2DArray, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray).
    */
    unsigned int    layer       = 0;

    /**
    \brief Cube texture face.
    \remarks This is only used for cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray).
    */
    AxisDirection   cubeFace    = AxisDirection::XPos;
};

//! Render target descriptor structure.
struct RenderTargetDescriptor
{
    //! Sampling descriptor.
    MultiSamplingDescriptor multiSampling;

    /**
    \brief Specifies whether custom multi-sampling is used or not. By default false.
    \remarks If this is true, only multi-sampled textures can be attached to a render-target,
    i.e. textures of the following types: Texture2DMS, Texture2DMSArray.
    If this is false, only non-multi-sampled textures can be attached to a render-target.
    This field will be ignored if multi-sampling is disabled.
    */
    bool                    customMultiSampling = false;
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
        \note A mixed attachment of multi-sample and non-multi-sample textures to a render-target is currently only supported with: Direct3D 11.
        */
        virtual void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) = 0;

        //! Detaches all textures and depth-stencil buffers from this render target.
        virtual void DetachAll() = 0;

        /**
        \brief Returns the render target resolution.
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
