/*
 * RenderPassFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_PASS_FLAGS_H
#define LLGL_RENDER_PASS_FLAGS_H


#include <LLGL/Format.h>
#include <LLGL/Constants.h>


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

    //! Store the outcome into the respective render-target attachment.
    Store,
};

/**
\brief Enumeration of modes how to resolve multi-sampled render-target attachments into their single-sampled resolve attachments.
\note Only supported with: Direct3D 12, Vulkan, and Metal. Supported modes depend on the backend, attachment type, and device.
\see AttachmentFormatDescriptor::resolveMode
*/
enum class ResolveMode
{
    /**
    \brief No resolve mode.
    \remarks Use this to render into a multi-sampled render-target with multiple render-passes
    where only the last one resolves the final result into a single-sampled render-target.
    */
    Disabled,

    /**
    \brief Resolve the minimum sample value of a multi-sampled render-target attachment into its respective resolve attachment.
    \note Vulkan supports this mode only for depth-stencil attachments and only when reported by the device.
    \note Metal supports this mode only for depth attachments on supported devices.
    \see RenderTargetDescriptor::resolveAttachments
    */
    Min,

    /**
    \brief Resolve the maximum sample value of a multi-sampled render-target attachment into its respective resolve attachment.
    \note Vulkan supports this mode only for depth-stencil attachments and only when reported by the device.
    \note Metal supports this mode only for depth attachments on supported devices.
    \see RenderTargetDescriptor::resolveAttachments
    */
    Max,

    /**
    \brief Resolve the average sample value of a multi-sampled render-target attachment into its respective resolve attachment.
    \remarks This can only be used with non-integer attachment formats, e.g. Format::R32Float, Format::RGBA8UNorm etc.
    Use Min, Max, or Optimal for integer attachment formats, e.g. Format::R32UInt or Format::D24UNormS8UInt.
    Vulkan color resolves use their native format-dependent behavior; explicit Average is not supported for integer color formats.
    Metal supports this mode only for non-integer color attachments.
    \see RenderTargetDescriptor::resolveAttachments
    \see IsIntegerFormat
    */
    Average,

    /**
    \brief Use the resolve mode that is optimal for the respective attachment format and rendering backend.
    \remarks For non-integer color formats (e.g. Format::R32Float, Format::RGBA8UNorm etc.), this maps to ResolveMode::Average.
    For integer color formats, the result is backend-dependent: Vulkan may select sample 0,
    Direct3D 12 may use the minimum or maximum, and OpenGL may select an implementation-dependent sample.
    Metal integer color resolves depend on device and format support.
    Depth-stencil resolves are an exception: Vulkan and Metal use sample 0, while Direct3D 12 uses Min.
    */
    Optimal,
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

    //! Constructor to initialize the format and optionally the load and store operations.
    inline AttachmentFormatDescriptor(
        const Format            format,
        const AttachmentLoadOp  loadOp      = AttachmentLoadOp::Load,
        const AttachmentStoreOp storeOp     = AttachmentStoreOp::Store,
        const ResolveMode       resolveMode = ResolveMode::Optimal)
    :
        format      { format      },
        loadOp      { loadOp      },
        storeOp     { storeOp     },
        resolveMode { resolveMode }
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
    Format                  format      = Format::Undefined;

    /**
    \brief Specifies the load operation of the previous attachment content. By default AttachmentLoadOp::Undefined.
    \remarks If the attachment is meant to be cleared when a render pass begins, set this to AttachmentLoadOp::Clear.
    \see AttachmentLoadOp
    */
    AttachmentLoadOp        loadOp      = AttachmentLoadOp::Undefined;

    /**
    \brief Specifies the store operation of the outcome for the respective attachment content. By default AttachmentStoreOp::Undefined.
    \see AttachmentStoreOp
    */
    AttachmentStoreOp       storeOp     = AttachmentStoreOp::Undefined;

    /**
    \brief Specifies the mode with which multi-sampled attachments will be resolved into their single-sampled attachments. By default ResolveMode::Optimal.
    \remarks This field is ignored if:
    - The render-pass has a sample count that is \e not greater than 1.
    - The respective resolve attachment in the render-target does \e not have a texture assigned.
    \see RenderPassDescriptor::samples
    \see RenderTargetDescriptor::resolveAttachments
    \see RenderTargetDescriptor::depthStencilResolveAttachment
    */
    ResolveMode             resolveMode = ResolveMode::Optimal;
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
    \brief Optional name for debugging purposes. By default null.
    \remarks The final name of the native hardware resource is implementation defined.
    \see RenderSystemChild::SetDebugName
    */
    const char*                 debugName           = nullptr;

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
    \remarks The depth and stencil attachments \b must have the same format unless one of them is Format::Undefined.
    They are listed separated here to specify independent load and store operations as well as resolve modes.
    */
    AttachmentFormatDescriptor  depthAttachment;

    /**
    \brief Specifies the stencil attachment used within the render pass.
    \remarks The depth and stencil attachments \b must have the same format unless one of them is Format::Undefined.
    They are listed separated here to specify independent load and store operations as well as resolve modes.
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

    /**
    \brief Specifies the number of views for multiview (single-pass layered) rendering. By default 1.
    \remarks If this is greater than 1, the render pass is a multiview render pass:
    a single draw command is broadcast to \c views consecutive array layers of the render target,
    starting at AttachmentDescriptor::arrayLayer. Each invocation can read its layer index via
    SystemValue::ViewIndex to index per-view data (e.g. view-projection matrices for single-pass stereo or
    cascaded shadow maps). All render target attachments used with this render pass must be array textures
    with at least <code>arrayLayer + views</code> layers.
    \remarks This must not be greater than RenderingLimits::maxViews and requires RenderingFeatures::hasMultiview.
    A value of 0 is treated the same as 1 (multiview disabled).
    \note Only supported with: Vulkan, Direct3D 12.
    \see RenderingFeatures::hasMultiview
    \see RenderingLimits::maxViews
    \see SystemValue::ViewIndex
    \see AttachmentDescriptor::arrayLayer
    */
    std::uint32_t               views               = 1;
};


} // /namespace LLGL


#endif



// ================================================================================
