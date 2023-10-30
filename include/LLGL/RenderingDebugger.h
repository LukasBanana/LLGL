/*
 * RenderingDebugger.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDERING_DEBUGGER_H
#define LLGL_RENDERING_DEBUGGER_H


#include <LLGL/Export.h>
#include <LLGL/Container/Strings.h>
#include <LLGL/Deprecated.h>
#include <vector>
#include <cstdint>
#include <cstring>


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
    const char*     annotation  = "";

    //! Elapsed time (in nanoseconds) to execute the respective command.
    std::uint64_t   elapsedTime = 0;
};

/**
\brief Profile of a rendered frame.
\see RenderingDebugger::NextFrame
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
        std::memset(values, 0, sizeof(values));
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
    \see RenderingDebugger::SetTimeRecording
    */
    std::vector<ProfileTimeRecord> timeRecords;
};


/* ----- Interfaces ----- */

/**
\brief Rendering debugger interface.
\remarks This can be used to profile the renderer draw calls and buffer updates.
*/
class LLGL_EXPORT RenderingDebugger
{

    public:

        //! Initializes the internal data.
        RenderingDebugger();

        //! Release the internal data.
        virtual ~RenderingDebugger();

        /**
        \brief Sets the new source function name.
        \param[in] source Pointer to a null terminated string that specifies the name. If this is null, the source is disabled.
        \note This function only stores the pointer. Hence, the pointer must be valid until a new pointer or a null pointer is set.
        */
        void SetSource(const char* source);

        /**
        \brief Sets the new debug group name.
        \param[in] name Pointer to a null terminated string that specifies the name. If this is null, the debug group is disabled.
        \note This function only stores the pointer. Hence, the pointer must be valid until a new pointer or a null pointer is set.
        */
        void SetDebugGroup(const char* name);

        /**
        \brief Enables or disbales time recording.
        \see FrameProfile::timeRecords
        */
        void SetTimeRecording(bool enabled);

        //! \retrun Returns whether time recording is enabled.
        bool GetTimeRecording() const;

        /**
        \brief Posts an error message.
        \param[in] type Specifies the type of error.
        \param[in] format Specifies the formatted message. Same as \c ::printf.
        */
        void Errorf(const ErrorType type, const char* format, ...);

        /**
        \brief Posts a warning message.
        \param[in] type Specifies the type of error.
        \param[in] format Specifies the formatted message. Same as \c ::printf.
        */
        void Warningf(const WarningType type, const char* format, ...);

        /**
        \brief Returns the current frame profile and resets the counters for the next frame.
        \param[out] outputProfile Optional pointer to an output profile to retrieve the current values. By default null.
        */
        void FlushProfile(FrameProfile* outputProfile = nullptr);

        /**
        \brief Records the specified profile with the current values.
        \param[in] profile Specifies the input profile whose values are to be merged with the current values.
        \see FrameProfile::Accumulate
        */
        void RecordProfile(const FrameProfile& profile);

        //! \deprecated Since 0.04b; Use Errorf instead!
        LLGL_DEPRECATED("LLGL::RenderingDebugger::PostError is deprecated since 0.04b; Use LLGL::RenderingDebugger::Errorf instead!", "Errorf")
        void PostError(const ErrorType type, const StringView& message);

        //! \deprecated Since 0.04b; Use Warningf instead!
        LLGL_DEPRECATED("LLGL::RenderingDebugger::PostWarning is deprecated since 0.04b; Use LLGL::RenderingDebugger::Warningf instead!", "Warningf")
        void PostWarning(const WarningType type, const StringView& message);

    protected:

        /**
        \brief Rendering debugger message class.
        \see RenderingDebugger::OnError
        \see RenderingDebugger::OnWarning
        */
        class LLGL_EXPORT Message
        {

            public:

                Message() = default;
                Message(const Message&) = default;
                Message& operator = (const Message&) = default;

                //! Initializes the message with text, source, and group name information.
                Message(const StringView& text, const StringView& source, const StringView& groupName);

                //! Blocks further occurrences of this message.
                void Block();

                //! Blocks further occurrences of this message after the specified amount of messages have been occurred.
                void BlockAfter(std::size_t occurrences);

                /**
                \brief Returns a report string for this message.
                \return Constructed report string containing all information of this message.
                */
                UTF8String ToReportString() const;

                //! Returns the message text.
                inline const UTF8String& GetText() const
                {
                    return text_;
                }

                //! Returns the source function where this message occurred.
                inline const UTF8String& GetSource() const
                {
                    return source_;
                }

                //! Returns the debug group name where this message occured.
                inline const UTF8String& GetGroupName() const
                {
                    return groupName_;
                }

                //! Returns the number of occurrences of this message.
                inline std::size_t GetOccurrences() const
                {
                    return occurrences_;
                }

                //! Returns true if this message has already been blocked.
                inline bool IsBlocked() const
                {
                    return blocked_;
                }

            protected:

                friend class RenderingDebugger;

                void IncOccurrence();

            private:

                UTF8String  text_;
                UTF8String  source_;
                UTF8String  groupName_;
                std::size_t occurrences_    = 1;
                bool        blocked_        = false;

        };

    protected:

        /**
        \brief Callback function when an error was posted.
        \remarks Use the 'message' parameter to block further occurences of this error if you like.
        The following example shows a custom implementation that is equivalent to the default implementation:
        \code
        class MyDebugger : public LLGL::RenderingDebugger {
            void OnError(ErrorType type, Message& message) override {
                LLGL::Log::Errorf(
                    "ERROR (" + std::string(LLGL::ToString(type)) + "): in '" + message.GetSource() + "': " + message.GetText()
                );
                message.Block();
            }
        };
        \endcode
        \see RenderingDebugger::PostError
        \see OnWarning
        */
        virtual void OnError(ErrorType type, Message& message);

        /**
        \brief Callback function when a warning was posted.
        \see RenderingDebugger::PostWarning
        \see OnError
        */
        virtual void OnWarning(WarningType type, Message& message);

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
