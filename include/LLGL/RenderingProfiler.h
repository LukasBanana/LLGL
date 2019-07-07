/*
 * RenderingProfiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDERING_PROFILER_H
#define LLGL_RENDERING_PROFILER_H


#include "Export.h"
#include "RenderContextFlags.h"
#include "GraphicsPipelineFlags.h"

#include <cstdint>
#include <algorithm>


namespace LLGL
{


/**
\brief Profile of a rendered frame.
\see RenderingProfiler::NextFrame
*/
struct FrameProfile
{
    //! Default constructor that initializes all counter values to zero.
    inline FrameProfile()
    {
        Clear();
    }

    //! Clears all counter values.
    inline void Clear()
    {
        std::fill(std::begin(values), std::end(values), 0);
    }

    //! Accumulates the specified profile with this profile.
    inline void Accumulate(const FrameProfile& rhs)
    {
        for (std::size_t i = 0; i < (sizeof(values) / sizeof(values[0])); ++i)
            values[i] += rhs.values[i];
    }

    union
    {
        struct
        {
            /**
            \brief Counter for all MIP-map generations.
            \see RenderSystem::GenerateMips
            */
            std::uint32_t mipMapsGenerations;

            /**
            \brief Counter for all vertex buffer and vertex buffer array bindings.
            \see CommandBuffer::SetVertexBuffer
            \see CommandBuffer::SetVertexBufferArray
            */
            std::uint32_t vertexBufferBindings;

            /**
            \brief Counter for all index buffer bindings.
            \see CommandBuffer::SetIndexBuffer
            */
            std::uint32_t indexBufferBindings;

            /**
            \brief Counter for all stream-output buffer and stream-output buffer array bindings.
            \see CommandBuffer::SetStreamOutputBuffer
            \see CommandBuffer::SetStreamOutputBufferArray
            */
            std::uint32_t streamOutputBufferBindings;

            /**
            \brief Counter for all individual constant buffer bindings (i.e. without a ResourceHeap).
            \see CommandBufferExt::SetConstantBuffer
            */
            std::uint32_t constantBufferBindings;

            /**
            \brief Counter for all individual sample buffer bindings (i.e. without a ResourceHeap).
            \see CommandBufferExt::SetSampleBuffer
            */
            std::uint32_t sampleBufferBindings;

            /**
            \brief Counter for all individual read/write storage buffer bindings (i.e. without a ResourceHeap).
            \see CommandBufferExt::SetRWStorageBuffer
            */
            std::uint32_t rwStorageBufferBindings;

            /**
            \brief Counter for all individual texture bindings (i.e. without a ResourceHeap).
            \see CommandBufferExt::SetTexture
            */
            std::uint32_t textureBindings;

            /**
            \brief Counter for all individual sampler bindings (i.e. without a ResourceHeap).
            \see CommandBufferExt::SetSampler
            */
            std::uint32_t samplerBindings;

            /**
            \brief Counter for all resource heap bindings on the graphics pipeline.
            \see CommandBuffer::SetGraphicsResourceHeap
            */
            std::uint32_t graphicsResourceHeapBindings;

            /**
            \brief Counter for all resource heap bindings on the compute pipeline.
            \see CommandBuffer::SetComputeResourceHeap
            */
            std::uint32_t computeResourceHeapBindings;

            /**
            \brief Counter for all graphics pipeline bindings.
            \see CommandBuffer::SetGraphicsPipeline
            */
            std::uint32_t graphicsPipelineBindings;

            /**
            \brief Counter for all compute pipeline bindings.
            \see CommandBuffer::SetComputePipeline
            */
            std::uint32_t computePipelineBindings;

            /**
            \brief Counter for all framebuffer attachment clear operations.
            \see CommandBuffer::Clear
            \see CommandBuffer::ClearAttachments
            */
            std::uint32_t attachmentClears;

            /**
            \brief Counter for all buffer updates during command encoding.
            \see CommandBuffer::UpdateBuffer
            */
            std::uint32_t bufferUpdates;

            /**
            \brief Counter for all buffer copies during command encoding.
            \see CommandBuffer::CopyBuffer
            */
            std::uint32_t bufferCopies;

            /**
            \brief Counter for all buffer write operations outside of command encoding.
            \see RenderSystem::WriteBuffer
            */
            std::uint32_t bufferWrites;

            /**
            \brief Counter for all buffer write operations outside of command encoding.
            \todo Not available yet.
            */
            std::uint32_t bufferReads;

            /**
            \brief Counter for all buffer map/unmap operations outside of command encoding.
            \see RenderSystem::MapBuffer.
            \see RenderSystem::UnmapBuffer.
            */
            std::uint32_t bufferMappings;

            /**
            \brief Counter for all texture copies during command encoding.
            \todo Not available yet.
            */
            std::uint32_t textureCopies;

            /**
            \brief Counter for all texture write operations outside of command encoding.
            \see RenderSystem::WriteTexture.
            */
            std::uint32_t textureWrites;

            /**
            \brief Counter for all texture write operations outside of command encoding.
            \see RenderSystem::ReadTexture.
            */
            std::uint32_t textureReads;

            /**
            \brief Counter for all texture write operations outside of command encoding.
            \todo Not available yet.
            */
            std::uint32_t textureMappings;

            /**
            \brief Counter for all command buffer sections that are enclosed by a call to \c BeginRenderPass and \c EndRenderPass.
            \see CommandBuffer::BeginRenderPass
            \see CommandBuffer::EndRenderPass
            */
            std::uint32_t renderPassSections;

            /**
            \brief Counter for all command buffer sections that are enclosed by a call to \c BeginStreamOutput and \c EndStreamOutput.
            \see CommandBuffer::BeginStreamOutput
            \see CommandBuffer::EndStreamOutput
            */
            std::uint32_t streamOutputSections;

            /**
            \brief Counter for all command buffer sections that are enclosed by a call to \c BeginQuery and \c EndQuery.
            \see CommandBuffer::BeginQuery
            \see CommandBuffer::EndQuery
            */
            std::uint32_t querySections;

            /**
            \brief Counter for all command buffer sections that are enclosed by a call to \c BeginRenderCondition and \c EndRenderCondition.
            \see CommandBuffer::BeginRenderCondition
            \see CommandBuffer::EndRenderCondition
            */
            std::uint32_t renderConditionSections;

            /**
            \brief Counter for all draw commands.
            \see CommandBuffer::Draw
            \see CommandBuffer::DrawIndexed
            \see CommandBuffer::DrawInstanced
            \see CommandBuffer::DrawIndexedInstanced
            */
            std::uint32_t drawCommands;

            /**
            \brief Counter for dispatch compute commands.
            \see CommandBuffer::Dispatch
            */
            std::uint32_t dispatchCommands;

            /**
            \brief Counter for all command buffers that were submitted to the queue.
            \see CommandQueue::Submit(CommandBuffer&)
            \see CommandQueue::Submit(std::uint32_t, CommandBuffer* const *)
            */
            std::uint32_t commandBufferSubmittions;

            /**
            \brief Counter for all command buffer encodings that are enclosed by a call to \c Begin and \c End.
            \see CommandBuffer::Begin
            \see CommandBuffer::End
            */
            std::uint32_t commandBufferEncodings;

            /**
            \brief Counter for all fences that were submitted to the queue.
            \see CommandQueue::Submit(Fence&)
            */
            std::uint32_t fenceSubmissions;
        };

        //! All proflile values as linear array.
        std::uint32_t values[32];
    };
};

/**
\brief Rendering profiler model class.
\remarks This can be used to profile the renderer draw calls and buffer updates.
\todo Refactor this for the new ResourceHeap and RenderPass interfaces.
*/
class LLGL_EXPORT RenderingProfiler
{

    public:

        /**
        \brief Returns the current frame profile and resets the counters for the next frame.
        \param[out] outputProfile Optional pointer to an output profile to retrieve the current values. By default null.
        */
        void NextProfile(FrameProfile* outputProfile = nullptr);

        /**
        \brief Accumulates the specified profile with the current values.
        \param[in] profile Specifies the input profile whose values are to be merged with the current values.
        \see FrameProfile::Accumulate
        */
        void Accumulate(const FrameProfile& profile);

        //! Current frame profile with all counter values.
        FrameProfile frameProfile;

};


} // /namespace LLGL


#endif



// ================================================================================
