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
#include <LLGL/RenderingDebuggerFlags.h>
#include <vector>
#include <cstdint>
#include <cstring>


namespace LLGL
{


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
        \see MergeProfiles
        */
        void RecordProfile(const FrameProfile& profile);

        //! \deprecated Since 0.04b; Use Errorf instead!
        LLGL_DEPRECATED("LLGL::RenderingDebugger::PostError is deprecated since 0.04b; Use LLGL::RenderingDebugger::Errorf instead!", "Errorf")
        void PostError(const ErrorType type, const StringView& message);

        //! \deprecated Since 0.04b; Use Warningf instead!
        LLGL_DEPRECATED("LLGL::RenderingDebugger::PostWarning is deprecated since 0.04b; Use LLGL::RenderingDebugger::Warningf instead!", "Warningf")
        void PostWarning(const WarningType type, const StringView& message);

    public:

        /**
        \brief Merges the source frame profile \c src into the destination frame profile \c dst.
        \see FrameProfile
        */
        static void MergeProfiles(FrameProfile& dst, const FrameProfile& src);

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

                //! Returns the debug group name where this message occurred.
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
        \remarks Use the 'message' parameter to block further occurrences of this error if you like.
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


inline void FrameProfile::Accumulate(const FrameProfile& rhs)
{
    RenderingDebugger::MergeProfiles(*this, rhs);
}


} // /namespace LLGL


#endif



// ================================================================================
