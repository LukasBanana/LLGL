/*
 * CommandBufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_FLAGS_H
#define LLGL_COMMAND_BUFFER_FLAGS_H


#include "ColorRGBA.h"


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

/**
\brief Pipeline binding point enumeration.
\see CommandBuffer::SetResourceHeap
*/
enum class PipelineBindPoint
{
    Undefined,  //!< Automatically determine pipeline binding point for a resource heap.
    Graphics,   //!< Graphics pipeline binding point.
    Compute,    //!< Compute pipeline binding point.
};


/* ----- Flags ----- */

/**
\brief Command buffer creation flags.
\see CommandBufferDescriptor::flags
*/
struct CommandBufferFlags
{
    enum
    {
        /**
        \brief Specifies that the encoded command buffer will be submitted as a secondary command buffer.
        \remarks If this is specified, the command buffer must be submitted using the \c Execute function of a primary command buffer.
        \see CommandBuffer::Execute
        \todo Rename to \c Secondary
        */
        DeferredSubmit  = (1 << 0),

        /**
        \brief Specifies that the encoded command buffer can be submitted multiple times.
        \remarks If this is not specified, the command buffer must be encoded again after it has been submitted to the command queue.
        \see CommandQueue::Submit(CommandBuffer&)
        \todo Rename to \c Restore
        */
        MultiSubmit     = (1 << 1),
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
    //! Specifies the clear value to clear a color attachment. By default (0.0, 0.0, 0.0, 0.0).
    ColorRGBAf      color   = { 0.0f, 0.0f, 0.0f, 0.0f };

    //! Specifies the clear value to clear a depth attachment. By default 1.0.
    float           depth   = 1.0f;

    //! Specifies the clear value to clear a stencil attachment. By default 0.
    std::uint32_t   stencil = 0;
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
    inline AttachmentClear(const ColorRGBAf& color, std::uint32_t colorAttachment) :
        flags           { ClearFlags::Color },
        colorAttachment { colorAttachment   }
    {
        clearValue.color = color;
    }

    //! Constructor for a depth attachment clear command.
    inline AttachmentClear(float depth) :
        flags { ClearFlags::Depth }
    {
        clearValue.depth = depth;
    }

    //! Constructor for a stencil attachment clear command.
    inline AttachmentClear(std::uint32_t stencil) :
        flags { ClearFlags::Stencil }
    {
        clearValue.stencil = stencil;
    }

    //! Constructor for a depth-stencil attachment clear command.
    inline AttachmentClear(float depth, std::uint32_t stencil) :
        flags { ClearFlags::DepthStencil }
    {
        clearValue.depth    = depth;
        clearValue.stencil  = stencil;
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
\brief Graphics API dependent state descriptor for the OpenGL renderer.
\remarks This descriptor is used to compensate a few differences between OpenGL and the other rendering APIs.
\see CommandBuffer::SetGraphicsAPIDependentState
\todo Move into namespace \c OpenGL
*/
struct OpenGLDependentStateDescriptor
{
    /**
    \brief Specifies whether the screen-space origin is on the lower-left. By default false.
    \remarks If this is true, the viewports and scissor rectangles of OpenGL are \b not emulated to the upper-left,
    which is the default to be uniform with other rendering APIs such as Direct3D and Vulkan.
    */
    bool originLowerLeft = false;

    /**
    \brief Specifies whether to invert front-facing. By default false.
    \remarks If this is true, the front facing (either \c GL_CW or \c GL_CCW) will be inverted,
    i.e. \c CCW becomes \c CW, and \c CW becomes \c CCW.
    */
    bool invertFrontFace = false;
};

/**
\brief Graphics API dependent state descriptor for the Metal renderer.
\remarks This descriptor is used to compensate a few differences between Metal and the other rendering APIs.
\see CommandBuffer::SetGraphicsAPIDependentState
\todo Move into namespace Metal
*/
struct MetalDependentStateDescriptor
{
    /**
    \brief Specifies the buffer slot for the internal tessellation factor buffer. By default 30, which is the maximum buffer slot.
    \remarks In the respective Metal tessellation kernel,
    this must refer to a buffer of type \c MTLTriangleTessellationFactorsHalf or \c MTLQuadTessellationFactorsHalf.
    */
    std::uint32_t tessFactorBufferSlot = 30;
};

/**
\brief Command buffer descriptor structure.
\see RenderSystem::CreateCommandBuffer
*/
struct CommandBufferDescriptor
{
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
};


} // /namespace LLGL


#endif



// ================================================================================
