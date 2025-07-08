/*
 * RenderingDebuggerFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDERING_DEBUGGER_FLAGS_H
#define LLGL_RENDERING_DEBUGGER_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Container/DynamicVector.h>
#include <LLGL/Container/StringLiteral.h>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Rendering debugger error types enumeration.
enum class ErrorType
{
    InvalidArgument,    //!< Error due to invalid argument (e.g. creating a graphics pipeline without a valid shader program being specified).
    InvalidState,       //!< Error due to invalid render state (e.g. rendering without a valid graphics pipeline).
    UnsupportedFeature, //!< Error due to use of unsupported feature (e.g. drawing with hardware instancing when the renderer hardware does not support it).
    UndefinedBehavior,  //!< Error due to arguments that cause undefined behavior.
};

//! Rendering debugger warning types enumeration.
enum class WarningType
{
    ImproperArgument,   //!< Warning due to improper argument (e.g. generating 4 vertices while having triangle list as primitive topology).
    ImproperState,      //!< Warning due to improper state (e.g. rendering while viewport is not visible).
    PointlessOperation, //!< Warning due to a operation without any effect (e.g. drawing with 0 vertices).
    VaryingBehavior,    //!< Warning due to a varying behavior between the native APIs (e.g. \c SV_VertexID in HLSL behaves different to \c gl_VertexID in GLSL or \c gl_VertexIndex in SPIRV).
};


/* ----- Structures ----- */

/**
\brief Structure with annotation and elapsed time for a timer profile.
\see FrameProfile::timeRecords
*/
struct ProfileTimeRecord
{
    //! Time record annotation, e.g. function name that was recorded from the CommandBuffer.
    StringLiteral   annotation;

    /**
    \brief CPU ticks at the beginning of the command.
    \see Timer::Tick
    */
    std::uint64_t   cpuTicksStart   = 0;

    /**
    \brief CPU ticks at the end of the command.
    \see Timer::Tick
    */
    std::uint64_t   cpuTicksEnd     = 0;

    /**
    \brief Elapsed time (in nanoseconds) to execute the respective command on the GPU.
    \remarks If no GPU time has been recorded for this command (e.g. for the record of debug groups), this value remains zero.
    */
    std::uint64_t   elapsedTime     = 0;
};

struct ProfileCommandQueueRecord
{
    /**
    \brief Counter for all buffer write operations outside of command encoding.
    \see RenderSystem::WriteBuffer
    */
    std::uint32_t bufferWrites              = 0;

    /**
    \brief Counter for all buffer write operations outside of command encoding.
    \see RenderSystem::ReadBuffer
    */
    std::uint32_t bufferReads               = 0;

    /**
    \brief Counter for all buffer map/unmap operations outside of command encoding.
    \see RenderSystem::MapBuffer.
    \see RenderSystem::UnmapBuffer.
    */
    std::uint32_t bufferMappings            = 0;

    /**
    \brief Counter for all texture write operations outside of command encoding.
    \see RenderSystem::WriteTexture.
    */
    std::uint32_t textureWrites             = 0;

    /**
    \brief Counter for all texture write operations outside of command encoding.
    \see RenderSystem::ReadTexture.
    */
    std::uint32_t textureReads              = 0;

    /**
    \brief Counter for all command buffers that were submitted to the queue.
    \see CommandQueue::Submit(CommandBuffer&)
    \see CommandQueue::Submit(std::uint32_t, CommandBuffer* const *)
    */
    std::uint32_t commandBufferSubmittions  = 0;

    /**
    \brief Counter for all fences that were submitted to the queue.
    \see CommandQueue::Submit(Fence&)
    */
    std::uint32_t fenceSubmissions          = 0;
};

struct ProfileCommandBufferRecord
{
    /**
    \brief Counter for all command buffer encodings that are enclosed by a call to \c Begin and \c End.
    \see CommandBuffer::Begin
    \see CommandBuffer::End
    */
    std::uint32_t encodings                 = 0;

    /**
    \brief Counter for all MIP-map generations.
    \see CommandBuffer::GenerateMips
    */
    std::uint32_t mipMapsGenerations        = 0;

    /**
    \brief Counter for all vertex buffer and vertex buffer array bindings.
    \see CommandBuffer::SetVertexBuffer
    \see CommandBuffer::SetVertexBufferArray
    */
    std::uint32_t vertexBufferBindings      = 0;

    /**
    \brief Counter for all index buffer bindings.
    \see CommandBuffer::SetIndexBuffer
    */
    std::uint32_t indexBufferBindings       = 0;

    /**
    \brief Counter for all individual constant buffer bindings.
    \see CommandBuffer::SetResource
    */
    std::uint32_t constantBufferBindings    = 0;

    /**
    \brief Counter for all sampled buffer bindings (i.e. with BindFlags::Sampled flag).
    \see CommandBuffer::SetResource
    */
    std::uint32_t sampledBufferBindings     = 0;

    /**
    \brief Counter for all storage buffer bindings (i.e. with BindFlags::Storage flag).
    \see CommandBuffer::SetResource
    */
    std::uint32_t storageBufferBindings     = 0;

    /**
    \brief Counter for all sampled texture bindings (i.e. with BindFlags::Sampled flag).
    \see CommandBuffer::SetResource
    */
    std::uint32_t sampledTextureBindings    = 0;

    /**
    \brief Counter for all sampled texture bindings (i.e. with BindFlags::Storage flag).
    \see CommandBuffer::SetResource
    */
    std::uint32_t storageTextureBindings    = 0;

    /**
    \brief Counter for all sampler-state bindings.
    \see CommandBuffer::SetResource
    */
    std::uint32_t samplerBindings           = 0;

    /**
    \brief Counter for all resource heap bindings.
    \see CommandBuffer::SetResourceHeap
    */
    std::uint32_t resourceHeapBindings      = 0;

    /**
    \brief Counter for all graphics pipeline state bindings.
    \see CommandBuffer::SetPipelineState
    */
    std::uint32_t graphicsPipelineBindings  = 0;

    /**
    \brief Counter for all compute pipeline state bindings.
    \see CommandBuffer::SetPipelineState
    */
    std::uint32_t computePipelineBindings   = 0;

    /**
    \brief Counter for all mesh pipeline state bindings.
    \see CommandBuffer::SetPipelineState
    */
    std::uint32_t meshPipelineBindings      = 0;

    /**
    \brief Counter for all framebuffer attachment clear operations.
    \see CommandBuffer::Clear
    \see CommandBuffer::ClearAttachments
    */
    std::uint32_t attachmentClears          = 0;

    /**
    \brief Counter for all buffer updates during command encoding.
    \see CommandBuffer::UpdateBuffer
    */
    std::uint32_t bufferUpdates             = 0;

    /**
    \brief Counter for all buffer copies during command encoding.
    \see CommandBuffer::CopyBuffer
    */
    std::uint32_t bufferCopies              = 0;

    /**
    \brief Counter for all buffer fills during command encoding.
    \see CommandBuffer::FillBuffer
    */
    std::uint32_t bufferFills               = 0;

    /**
    \brief Counter for all texture copies during command encoding.
    \see CommandBuffer::CopyTexture.
    */
    std::uint32_t textureCopies             = 0;

    /**
    \brief Counter for all command buffer sections that are enclosed by a call to \c BeginRenderPass and \c EndRenderPass.
    \see CommandBuffer::BeginRenderPass
    \see CommandBuffer::EndRenderPass
    */
    std::uint32_t renderPassSections        = 0;

    /**
    \brief Counter for all command buffer sections that are enclosed by a call to \c BeginStreamOutput and \c EndStreamOutput.
    \see CommandBuffer::BeginStreamOutput
    \see CommandBuffer::EndStreamOutput
    */
    std::uint32_t streamOutputSections      = 0;

    /**
    \brief Counter for all command buffer sections that are enclosed by a call to \c BeginQuery and \c EndQuery.
    \see CommandBuffer::BeginQuery
    \see CommandBuffer::EndQuery
    */
    std::uint32_t querySections             = 0;

    /**
    \brief Counter for all command buffer sections that are enclosed by a call to \c BeginRenderCondition and \c EndRenderCondition.
    \see CommandBuffer::BeginRenderCondition
    \see CommandBuffer::EndRenderCondition
    */
    std::uint32_t renderConditionSections   = 0;

    /**
    \brief Counter for all draw commands.
    \see CommandBuffer::Draw
    \see CommandBuffer::DrawIndexed
    \see CommandBuffer::DrawInstanced
    \see CommandBuffer::DrawIndexedInstanced
    \see CommandBuffer::DrawIndirect
    \see CommandBuffer::DrawIndexedIndirect
    \see CommandBuffer::DrawStreamOutput
    */
    std::uint32_t drawCommands              = 0;

    /**
    \brief Counter for dispatch compute commands.
    \see CommandBuffer::Dispatch
    \see CommandBuffer::DispatchIndirect
    */
    std::uint32_t dispatchCommands          = 0;

    /**
    \brief Counter for mesh draw commands.
    \see CommandBufferTier1::DrawMesh
    \see CommandBufferTier1::DrawMeshIndirect
    */
    std::uint32_t meshCommands              = 0;
};

/**
\brief Profile of a rendered frame.
\see RenderingDebugger::NextFrame
*/
struct FrameProfile
{
    /**
    \brief Structure for all command queue recordings of this frame profile.
    \remarks This also includes internal queue submissions from the RenderSystem.
    see ProfileCommandQueueRecord
    */
    ProfileCommandQueueRecord           commandQueueRecord;

    /**
    \brief Structure for all command buffer recordings of this frame profile.
    see ProfileCommandBufferRecord
    */
    ProfileCommandBufferRecord          commandBufferRecord;

    /**
    \brief List of all time records for this frame profile.
    \see RenderingDebugger::SetTimeRecording
    */
    DynamicVector<ProfileTimeRecord>    timeRecords;
};


} // /namespace LLGL


#endif



// ================================================================================
