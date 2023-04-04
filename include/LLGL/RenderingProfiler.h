/*
 * RenderingProfiler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDERING_PROFILER_H
#define LLGL_RENDERING_PROFILER_H


#include <LLGL/SwapChainFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <cstdint>
#include <string.h>


namespace LLGL
{


/**
\brief Structure with annotation and elapsed time for a timer profile.
\see FrameProfile::timeRecords
*/
struct ProfileTimeRecord
{
    //! Time record annotation, e.g. function name that was recorded from the CommandBuffer.
    const char*     annotation  = "";

    //! Elapsed time (in nanoseconds) to execute the respective command.
    std::uint64_t   elapsedTime = 0;
};

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
        ::memset(values, 0, sizeof(values));
        timeRecords.clear();
    }

    //! Accumulates the specified profile with this profile.
    inline void Accumulate(const FrameProfile& rhs)
    {
        /* Accumulate counters */
        for (std::size_t i = 0; i < (sizeof(values) / sizeof(values[0])); ++i)
            values[i] += rhs.values[i];

        /* Append time records */
        timeRecords.insert(timeRecords.end(), rhs.timeRecords.begin(), rhs.timeRecords.end());
    }

    union
    {
        struct
        {
            /**
            \brief Counter for all MIP-map generations.
            \see CommandBuffer::GenerateMips
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
            \brief Counter for all individual constant buffer bindings.
            \see CommandBuffer::SetResource
            */
            std::uint32_t constantBufferBindings;

            /**
            \brief Counter for all sampled buffer bindings (i.e. with BindFlags::Sampled flag).
            \see CommandBuffer::SetResource
            */
            std::uint32_t sampledBufferBindings;

            /**
            \brief Counter for all storage buffer bindings (i.e. with BindFlags::Storage flag).
            \see CommandBuffer::SetResource
            */
            std::uint32_t storageBufferBindings;

            /**
            \brief Counter for all sampled texture bindings (i.e. with BindFlags::Sampled flag).
            \see CommandBuffer::SetResource
            */
            std::uint32_t sampledTextureBindings;

            /**
            \brief Counter for all sampled texture bindings (i.e. with BindFlags::Storage flag).
            \see CommandBuffer::SetResource
            */
            std::uint32_t storageTextureBindings;

            /**
            \brief Counter for all sampler-state bindings.
            \see CommandBuffer::SetResource
            */
            std::uint32_t samplerBindings;

            /**
            \brief Counter for all resource heap bindings.
            \see CommandBuffer::SetResourceHeap
            */
            std::uint32_t resourceHeapBindings;

            /**
            \brief Counter for all graphics pipeline state bindings.
            \see CommandBuffer::SetPipelineState
            */
            std::uint32_t graphicsPipelineBindings;

            /**
            \brief Counter for all compute pipeline state bindings.
            \see CommandBuffer::SetPipelineState
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
            \brief Counter for all buffer fills during command encoding.
            \see CommandBuffer::FillBuffer
            */
            std::uint32_t bufferFills;

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
            \see CommandBuffer::CopyTexture.
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
        std::uint32_t values[33];
    };

    /**
    \brief List of all time records for this frame profile.
    \see RenderingProfiler::timeRecordingEnabled
    */
    std::vector<ProfileTimeRecord> timeRecords;
};

/**
\brief Rendering profiler model class.
\remarks This can be used to profile the renderer draw calls and buffer updates.
\todo Refactor this for the new ResourceHeap and RenderPass interfaces.
*/
class RenderingProfiler
{

    public:

        /**
        \brief Returns the current frame profile and resets the counters for the next frame.
        \param[out] outputProfile Optional pointer to an output profile to retrieve the current values. By default null.
        */
        inline void NextProfile(FrameProfile* outputProfile = nullptr)
        {
            /* Copy current counters to the output profile (if set) */
            if (outputProfile)
                *outputProfile = frameProfile;

            /* Clear values */
            frameProfile.Clear();
        }

        /**
        \brief Accumulates the specified profile with the current values.
        \param[in] profile Specifies the input profile whose values are to be merged with the current values.
        \see FrameProfile::Accumulate
        */
        inline void Accumulate(const FrameProfile& profile)
        {
            frameProfile.Accumulate(profile);
        }

    public:

        //! Current frame profile with all counter values.
        FrameProfile    frameProfile;

        /**
        \brief Specifis whether the command buffer time recording is enabled or disabled. By default disabled.
        \see FrameProfile::timeRecords
        */
        bool            timeRecordingEnabled    = false;

};


} // /namespace LLGL


#endif



// ================================================================================
