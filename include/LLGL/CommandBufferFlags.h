/*
 * CommandBufferFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMMAND_BUFFER_FLAGS_H
#define LLGL_COMMAND_BUFFER_FLAGS_H


#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Render condition mode enumeration.
\remarks The condition is determined by the type of the QueryHeap object.
\see CommandBuffer::BeginRenderCondition
*/
enum class RenderConditionMode
{
    Wait,                   //!< Wait until the occlusion query result is available, before conditional rendering begins.
    NoWait,                 //!< Do not wait until the occlusion query result is available, before conditional rendering begins.
    ByRegionWait,           //!< Similar to Wait, but the renderer may discard the results of commands for any framebuffer region that did not contribute to the occlusion query.
    ByRegionNoWait,         //!< Similar to NoWait, but the renderer may discard the results of commands for any framebuffer region that did not contribute to the occlusion query.
    WaitInverted,           //!< Same as Wait, but the condition is inverted.
    NoWaitInverted,         //!< Same as NoWait, but the condition is inverted.
    ByRegionWaitInverted,   //!< Same as ByRegionWait, but the condition is inverted.
    ByRegionNoWaitInverted, //!< Same as ByRegionNoWait, but the condition is inverted.
};

/**
\brief Stencil face enumeration.
\remarks To be compatible with Direct3D renderers, it is recommended to always use \c FrontAndBack.
\see CommandBuffer::SetStencilReference
*/
enum class StencilFace
{
    //! Refers to both the front and back face of primitives.
    FrontAndBack,

    /**
    \brief Refers only to the front face of primitives.
    \note Only supported with: OpenGL, Vulkan, Metal.
    */
    Front,

    /**
    \brief Refers only to the back face of primitives.
    \note Only supported with: OpenGL, Vulkan, Metal.
    */
    Back,
};


/* ----- Flags ----- */

/**
\brief Command buffer creation flags.
\remarks A default command buffer is a primary command buffer (No \c Secondary flag) that has to be submitted explicitly (No \c ImmediateSubmit flag)
and can only be submitted once until it is encoded again (No \c MultiSubmit flag).
\see CommandBufferDescriptor::flags
*/
struct CommandBufferFlags
{
    enum
    {
        /**
        \brief Specifies that the encoded command buffer will be submitted as a secondary command buffer.
        \remarks If this is specified, the command buffer must be submitted using the \c Execute function of a primary command buffer.
        \remarks This cannot be used in combination with the \c ImmediateSubmit flag.
        \see CommandBuffer::Execute
        */
        Secondary       = (1 << 0),

        /**
        \brief Specifies that the encoded command buffer can be submitted multiple times.
        \remarks If this is not specified, the command buffer must be encoded again after it has been submitted to the command queue.
        \remarks This cannot be used in combination with the \c ImmediateSubmit flag.
        \see CommandQueue::Submit(CommandBuffer&)
        */
        MultiSubmit     = (1 << 1),

        /**
        \brief Specifies that the encoded command buffer is an immediate command buffer.
        \remarks If this is specified, the command buffer is submitted immediately after encoding is done
        and calling CommandQueue::Submit on such a command buffer has no effect.
        \remarks This cannot be used in combination with the \c Secondary or \c MultiSubmit flags.
        \see CommandBuffer::End
        */
        ImmediateSubmit = (1 << 2),
    };
};

/**
\brief Command buffer clear flags.
\see CommandBuffer::Clear
*/
struct ClearFlags
{
    enum
    {
        Color           = (1 << 0),                     //!< Clears the color attachment.
        Depth           = (1 << 1),                     //!< Clears the depth attachment.
        Stencil         = (1 << 2),                     //!< Clears the stencil attachment.

        ColorDepth      = (Color | Depth),              //!< Clears the color and depth attachments.
        DepthStencil    = (Depth | Stencil),            //!< Clears the depth and stencil attachments.
        All             = (Color | Depth | Stencil),    //!< Clears the color, depth and stencil attachments.
    };
};


/* ----- Structures ----- */

/**
\brief Clear value structure for color, depth, and stencil clear operations.
\see AttachmentClear::clearValue
\see TextureDescriptor::clearValue
*/
struct ClearValue
{
    ClearValue() = default;
    ClearValue(const ClearValue&) = default;
    ClearValue& operator = (const ClearValue&) = default;

    //! Constructor for color, depth, and stencil values.
    inline ClearValue(const float color[4], float depth = 1.0f, std::uint32_t stencil = 0) :
        color   { color[0], color[1], color[2], color[3] },
        depth   { depth                                  },
        stencil { stencil                                }
    {
    }

    //! Constructor for color, depth, and stencil values.
    inline ClearValue(float r, float g, float b, float a, float depth = 1.0f, std::uint32_t stencil = 0) :
        color   { r, g, b, a },
        depth   { depth      },
        stencil { stencil    }
    {
    }

    //! Constructor for the depth value only.
    inline ClearValue(float depth) :
        depth { depth }
    {
    }

    //! Constructor for the stencil value only.
    inline ClearValue(std::uint32_t stencil) :
        stencil { stencil }
    {
    }

    //! Constructor for the depth and stencil values.
    inline ClearValue(float depth, std::uint32_t stencil) :
        depth   { depth   },
        stencil { stencil }
    {
    }

    //! Specifies the clear value to clear a color attachment. By default (0.0, 0.0, 0.0, 0.0).
    float           color[4]    = { 0.0f, 0.0f, 0.0f, 0.0f };

    //! Specifies the clear value to clear a depth attachment. By default 1.0.
    float           depth       = 1.0f;

    //! Specifies the clear value to clear a stencil attachment. By default 0.
    std::uint32_t   stencil     = 0;
};

/**
\brief Attachment clear command structure.
\see CommandBuffer::ClearAttachments
*/
struct AttachmentClear
{
    AttachmentClear() = default;
    AttachmentClear(const AttachmentClear&) = default;
    AttachmentClear& operator = (const AttachmentClear&) = default;

    //! Constructor for a color attachment clear command.
    inline AttachmentClear(const float color[4], std::uint32_t colorAttachment) :
        flags           { ClearFlags::Color },
        colorAttachment { colorAttachment   },
        clearValue      { color             }
    {
    }

    //! Constructor for a depth attachment clear command.
    inline AttachmentClear(float depth) :
        flags      { ClearFlags::Depth },
        clearValue { depth             }
    {
    }

    //! Constructor for a stencil attachment clear command.
    inline AttachmentClear(std::uint32_t stencil) :
        flags      { ClearFlags::Stencil },
        clearValue { stencil             }
    {
    }

    //! Constructor for a depth-stencil attachment clear command.
    inline AttachmentClear(float depth, std::uint32_t stencil) :
        flags      { ClearFlags::DepthStencil },
        clearValue { depth, stencil           }
    {
    }

    /**
    \brief Specifies the clear buffer flags.
    \remarks This can be a bitwise OR combination of the "ClearFlags" enumeration entries.
    However, if the ClearFlags::Color bit is set, all other bits are ignored.
    It is recommended to clear depth- and stencil buffers always simultaneously if both are meant to be cleared (i.e. use ClearFlags::DepthStencil in this case).
    \see ClearFlags
    */
    long            flags           = 0;

    /**
    \brief Specifies the index of the color attachment within the active render target. By default 0.
    \remarks This is ignored if the ClearFlags::Color bit is not set in the 'flags' member.
    \see flags
    */
    std::uint32_t   colorAttachment = 0;

    //! Clear value for color, depth, and stencil buffers.
    ClearValue      clearValue;
};

/**
\brief Command buffer descriptor structure.
\see RenderSystem::CreateCommandBuffer
*/
struct CommandBufferDescriptor
{
    CommandBufferDescriptor() = default;
    CommandBufferDescriptor(const CommandBufferDescriptor&) = default;
    CommandBufferDescriptor& operator = (const CommandBufferDescriptor&) = default;

    //! Constructs the command buffer descriptor with the specified flags.
    inline CommandBufferDescriptor(long flags) :
        flags { flags }
    {
    }

    //! Constructs the command buffer descriptor with the specified flags and number of native buffers.
    inline CommandBufferDescriptor(long flags, std::uint32_t numNativeBuffers) :
        flags            { flags            },
        numNativeBuffers { numNativeBuffers }
    {
    }

    /**
    \brief Specifies the creation flags for the command buffer. By default 0.
    \remarks If no flags are specified (i.e. the default value),
    the command buffer must be encoded again after it has been submitted to the command queue.
    \see CommandBufferFlags
    */
    long            flags               = 0;

    /**
    \brief Specifies the number of internal native command buffers. By default 2.
    \remarks This is only a hint to the framework, since not all rendering APIs support command buffers natively.
    For those that do, however, this member specifies how many native command buffers are to be allocated internally.
    These native command buffers are then switched everytime encoding begins with the CommandBuffer::Begin function.
    The benefit of having multiple native command buffers is that it reduces the time the GPU is idle
    because it waits for a command buffer to be completed before it can be reused.
    \see CommandBuffer::Begin
    */
    std::uint32_t   numNativeBuffers    = 2;

    /**
    \brief Specifies the minimum size (in bytes) for the staging pool (if supported). By default 65536 (or <tt>0xFFFF + 1</tt>).
    \remarks This is only a hint to the framework, since not all rendering APIs support command buffers natively.
    For the D3D12 backend for instance, this will specify the initial buffer size for the staging pool, i.e. for buffer updates during command encoding.
    For command buffers that will make many and large buffer updates, increase this size to fine-tune performance.
    \see CommandBuffer::UpdateBuffer
    */
    std::uint64_t   minStagingPoolSize  = (0xFFFF + 1);
};


} // /namespace LLGL


#endif



// ================================================================================
