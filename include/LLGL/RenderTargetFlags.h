/*
 * RenderTargetFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_TARGET_FLAGS_H
#define LLGL_RENDER_TARGET_FLAGS_H


#include "TextureFlags.h"
#include "GraphicsPipelineFlags.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class Texture;

/* ----- Enumerations ----- */

/**
\brief Render target attachment type enumeration.
\see AttachmentDescriptor
*/
enum class AttachmentType
{
    Color,          //!< Attachment is used for color output.
    Depth,          //!< Attachment is used for depth component output.
    DepthStencil,   //!< Attachment is used for depth component and stencil index output.
    Stencil,        //!< Attachment is used for stencil index output.
};


/* ----- Structures ----- */

/**
\brief Render target attachment descriptor structure.
\see RenderTargetDescriptor
*/
struct AttachmentDescriptor
{
    AttachmentDescriptor() = default;
    AttachmentDescriptor(const AttachmentDescriptor&) = default;

    //! Constructor for the specified depth-, stencil-, or color attachment.
    inline AttachmentDescriptor(AttachmentType type, Texture* texture, std::uint32_t mipLevel = 0, std::uint32_t layer = 0, AxisDirection cubeFace = AxisDirection::XPos) :
        type     { type     },
        texture  { texture  },
        mipLevel { mipLevel },
        layer    { layer    },
        cubeFace { cubeFace }
    {
    }

    //! Constructor for the specified depth-, or stencil attachment.
    inline AttachmentDescriptor(AttachmentType type, std::uint32_t width, std::uint32_t height) :
        type   { type   },
        width  { width  },
        height { height }
    {
    }

    /**
    \brief Specifies for which output information the texture attachment is to be used,
    e.g. for color or depth information. By default AttachmentType::Color.
    */
    AttachmentType  type        = AttachmentType::Color;

    /**
    \brief Pointer to the texture which is to be used as target output. By default null.
    \remarks If this is null, the attribute 'type' must not be AttachmentType::Color and the attributes 'width' and 'height' must be specified.
    The texture must also have been created with the flag 'TextureFlags::AttachmentUsage'.
    \see AttachmentDescriptor::type
    \see TextureFlags::AttachmentUsage
    */
    Texture*        texture     = nullptr;

    #if 0//TODO

    /**

    */
    Extent2D        resolution;

    #else

    /**
    \brief Specifies the width of the attachment resolution.
    \remarks If 'texture' is a valid pointer to a Texture object, this value is ignored and the required resolution is determined by that texture object.
    \todo Maybe combine width and height with new attribute "Extent2D extent".
    */
    std::uint32_t   width       = 0;

    /**
    \brief Specifies the height of the attachment resolution.
    \remarks If 'texture' is a valid pointer to a Texture object, this value is ignored and the required resolution is determined by that texture object.
    \todo Maybe combine width and height with new attribute "Extent2D extent".
    */
    std::uint32_t   height      = 0;

    #endif

    /**
    \brief Specifies the MIP-map level which is to be attached to a render target.
    \remarks This is only used for non-multi-sample textures.
    All multi-sample textures will always use the first MIP-map level
    (i.e. TextureType::Texture2DMS and TextureType::Texture2DMSArray).
    */
    std::uint32_t   mipLevel    = 0;

    /**
    \brief Specifies the array texture layer which is to be used as render target attachment.
    \remarks This is only used for array textures (i.e. TextureType::Texture1DArray,
    TextureType::Texture2DArray, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray).
    For cube array textures this can be used in combination with the cubeFace attribute.
    \see cubeFace
    */
    std::uint32_t   layer       = 0;

    /**
    \brief Cube texture face.
    \remarks This is only used for cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray).
    */
    AxisDirection   cubeFace    = AxisDirection::XPos;
};

/**
\brief Render target descriptor structure.
\remarks Here is a small example of a render target descriptor with a color attachmnet
and an anonymous depth attachment (i.e. without a texture reference, which is only allowed for depth/stencil attachments):
\code
LLGL::RenderTargetDescriptor rtDesc;
{
    rtDesc.attachments.resize(2);

    rtDesc.attachments[0].type      = LLGL::AttachmentType::Color;
    rtDesc.attachments[0].texture   = colorTexture;

    rtDesc.attachments[1].type      = LLGL::AttachmentType::Depth;
    rtDesc.attachments[1].width     = colorTextureWidth;
    rtDesc.attachments[1].height    = colorTextureHeight;
}
auto renderTarget = renderer->CreateRenderTarget(rtDesc);
\endcode
*/
struct RenderTargetDescriptor
{
    /**
    \brief Specifies all render target attachment descriptors.
    \remarks This must contain at least one entry.
    */
    std::vector<AttachmentDescriptor>   attachments;

    //! Multi-sampling descriptor. By default, multi-sampling is disabled.
    MultiSamplingDescriptor             multiSampling;

    /**
    \brief Specifies whether custom multi-sampling is used or not. By default false.
    \remarks If this is true, only multi-sampled textures can be attached to a render-target,
    i.e. textures of the following types: Texture2DMS, Texture2DMSArray.
    If this is false, only non-multi-sampled textures can be attached to a render-target.
    This field will be ignored if multi-sampling is disabled.
    */
    bool                                customMultiSampling = false;
};


} // /namespace LLGL


#endif



// ================================================================================
