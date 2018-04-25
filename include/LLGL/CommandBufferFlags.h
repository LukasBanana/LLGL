/*
 * CommandBufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_FLAGS_H
#define LLGL_COMMAND_BUFFER_FLAGS_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Render condition mode enumeration.
\remarks The condition is determined by the type of the Query object.
\see RenderContext::BeginRenderCondition
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


/* ----- Structures ----- */

/**
\brief Command buffer clear flags.
\see CommandBuffer::Clear
*/
struct ClearFlags
{
    enum
    {
        Color           = (1 << 0),                     //!< Clears the color buffer.
        Depth           = (1 << 1),                     //!< Clears the depth buffer.
        Stencil         = (1 << 2),                     //!< Clears the stencil buffer.

        ColorDepth      = (Color | Depth),              //!< Clears the color and depth buffers.
        DepthStencil    = (Depth | Stencil),            //!< Clears the depth and stencil buffers.
        All             = (Color | Depth | Stencil),    //!< Clears the all buffers (color, depth, and stencil).
    };
};

/**
\brief Low-level graphics API dependent state descriptor union.
\remarks This descriptor is used to compensate a few differences between OpenGL and Direct3D.
\see RenderContext::SetGraphicsAPIDependentState
*/
union GraphicsAPIDependentStateDescriptor
{
    GraphicsAPIDependentStateDescriptor()
    {
        stateOpenGL.screenSpaceOriginLowerLeft      = false;
        stateOpenGL.invertFrontFace                 = false;

        stateDirect3D12.disableAutoStateSubmission  = false;
    }

    struct StateOpenGLDescriptor
    {
        /**
        \brief Specifies whether the screen-space origin is on the lower-left. By default false.
        \remarks If this is true, the viewports and scissor rectangles of OpenGL are NOT emulated to the upper-left,
        which is the default to be uniform with other rendering APIs such as Direct3D and Vulkan.
        */
        bool        screenSpaceOriginLowerLeft;

        /**
        \brief Specifies whether to invert front-facing. By default false.
        \remarks If this is true, the front facing (either GL_CW or GL_CCW) will be inverted,
        i.e. CCW becomes CW, and CW becomes CCW.
        */
        bool        invertFrontFace;
    }
    stateOpenGL;

    //! \todo Remove this as soon as SetViewport and SetScissor are replaced by graphics pipeline states.
    struct StateDirect3D12Descriptor
    {
        /**
        \brief Specifies whether persistent states are automatically submitted to the command buffer or not. By default false.
        \remarks If this is true, "SetViewport" or "SetViewportArray", and "SetScissor" or "SetScissorArray" of the CommandBuffer interface
        must be called every time after the command buffer has been submitted to the command queue (e.g. after the "RenderContext::Present" function has been called).
        \see CommandBuffer::SetViewport
        \see CommandBuffer::SetViewportArray
        \see CommandBuffer::SetScissor
        \see CommandBuffer::SetScissorArray
        \deprecated Will be replaced by GraphicsPipelineDescriptor::viewports and CommandBuffer::SetViewport.
        */
        bool        disableAutoStateSubmission;
    }
    stateDirect3D12;
};


} // /namespace LLGL


#endif



// ================================================================================
