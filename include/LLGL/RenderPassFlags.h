/*
 * RenderPassFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_PASS_FLAGS_H
#define LLGL_RENDER_PASS_FLAGS_H


#include <LLGL/Format.h>
#include <LLGL/StaticLimits.h>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Enumeration for render pass attachment load operations.
\see AttachmentFormatDescriptor
*/
enum class AttachmentLoadOp
{
    //! We don't care about the previous content of the respective render target attachment.
    Undefined,

    //! Loads the previous content of the respective render target attachment.
    Load,

    /**
    \brief Clear the previous content of the respective render target attachment.
    \remarks The clear value used for this load operation is specified at the CommandBuffer::BeginRenderPass function.
    \see CommandBuffer::BeginRenderPass
    */
    Clear,
};

/**
\brief Enumeration for render pass attachment store operations.
\see AttachmentFormatDescriptor
*/
enum class AttachmentStoreOp
{
    /**
    \brief We don't care about the outcome of the respective render target attachment.
    \remarks Can be used, for example, if we only need the depth buffer for the depth test, but nothing is written to it.
    */
    Undefined,

    //! Stores the outcome in the respective render target attachment.
    Store,
};


/* ----- Structures ----- */

/**
\brief Render pass attachment descriptor structure.
\remarks Two attachment format descriptors are considered compatible when their formats and multi-sampling attributes are matching.
\see RenderPassDescriptor
*/
struct AttachmentFormatDescriptor
{
    AttachmentFormatDescriptor() = default;
    AttachmentFormatDescriptor(const AttachmentFormatDescriptor&) = default;

    //! Constructor to initialize the format and optionally the load and store operations.
    inline AttachmentFormatDescriptor(
        const Format            format,
        const AttachmentLoadOp  loadOp  = AttachmentLoadOp::Load,
        const AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
    :
        format  { format  },
        loadOp  { loadOp  },
        storeOp { storeOp }
    {
    }

    /**
    \brief Specifies the render target attachment format. By default Format::Undefined.
    \remarks If the render pass is used for a swap-chain, the appropriate color format can be determined by the SwapChain::GetColorFormat function,
    and the appropriate depth-stencil format can be determined by the SwapChain::GetDepthStencilFormat function.
    If the render pass is used for render targets, the format depends on the render target attachments.
    If this is undefined, the corresponding attachment is not used.
    \see SwapChain::GetColorFormat
    \see SwapChain::GetDepthStencilFormat
    */
    Format                  format  = Format::Undefined;

    /**
    \brief Specifies the load operation of the previous attachment content. By default AttachmentLoadOp::Undefined.
    \remarks If the attachment is meant to be cleared when a render pass begins, set this to AttachmentLoadOp::Clear.
    \see AttachmentLoadOp
    */
    AttachmentLoadOp        loadOp  = AttachmentLoadOp::Undefined;

    /**
    \brief Specifies the store operation of the outcome for the respective attachment content. By default AttachmentStoreOp::Undefined.
    \see AttachmentStoreOp
    */
    AttachmentStoreOp       storeOp = AttachmentStoreOp::Undefined;
};

/**
\brief Render pass descriptor structure.
\remarks A render pass object can be used across multiple render targets.
Moreover, a render target can be created with a different render pass object than the one used for CommandBuffer::BeginRenderPass as long as they are compatible.
Two render passes are considered compatible when all color-, depth-, and stencil attachments are compatible.
\see RenderSystem::CreateRenderPass
\see CommandBuffer::BeginRenderPass
\see AttachmentFormatDescriptor
*/
struct RenderPassDescriptor
{
    /**
    \brief Specifies the color attachments used within the render pass.
    \remarks Each attachment with a \c format field set to Format::Undefined is disabled.
    Any other attachment after the first disabled attachment is also considered disabled as attachments must be enabled in consecutive order.
    A swap-chain usually uses a BGRA format instead of an RGBA format.
    \see RenderingLimits::maxColorAttachments
    \see Format::BGRA8UNorm
    \see Format::BGRA8sRGB
    */
    AttachmentFormatDescriptor  colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    /**
    \brief Specifies the depth attachment used within the render pass.
    \remarks The depth attachment and stencil attachment usually share the same format (e.g. Format::D24UNormS8UInt).
    They are separated here to specify different load and store operations.
    */
    AttachmentFormatDescriptor  depthAttachment;

    /**
    \brief Specifies the stencil attachment used within the render pass.
    \remarks The depth attachment and stencil attachment usually share the same format (e.g. Format::D24UNormS8UInt).
    They are separated here to specify different load and store operations.
    */
    AttachmentFormatDescriptor  stencilAttachment;

    /**
    \brief Specifies the number of samples for the respective render target attachment. By default 1.
    \remarks This must be greater than 0. If this is 1, multi-sampling is disabled.
    All attachments and the respective render target must have the same number of samples.
    \see TextureDescriptor::samples
    \see RenderingLimits::maxColorBufferSamples
    \see RenderingLimits::maxDepthBufferSamples
    \see RenderingLimits::maxStencilBufferSamples
    \see RenderingLimits::maxNoAttachmentSamples
    */
    std::uint32_t               samples             = 1;
};


} // /namespace LLGL


#endif



// ================================================================================
