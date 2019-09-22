/*
 * RenderingDebugger.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDERING_DEBUGGER_H
#define LLGL_RENDERING_DEBUGGER_H


#include "Export.h"
#include <map>
#include <string>


namespace LLGL
{


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


/**
\brief Rendering debugger interface.
\remarks This can be used to profile the renderer draw calls and buffer updates.
*/
class LLGL_EXPORT RenderingDebugger
{

    public:

        virtual ~RenderingDebugger() = default;

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
        \brief Posts an error message.
        \param[in] type Specifies the type of error.
        \param[in] message Specifies the string which describes the failure.
        */
        void PostError(const ErrorType type, const std::string& message);

        /**
        \brief Posts a warning message.
        \param[in] type Specifies the type of error.
        \param[in] message Specifies the string which describes the warning.
        */
        void PostWarning(const WarningType type, const std::string& message);

    protected:

        /**
        \brief Rendering debugger message class.
        \todo Rename to "Report"
        */
        class LLGL_EXPORT Message
        {

            public:

                Message() = default;
                Message(const Message&) = default;
                Message& operator = (const Message&) = default;

                Message(const std::string& text, const std::string& source, const std::string& groupName);

                //! Blocks further occurrences of this message.
                void Block();

                //! Blocks further occurrences of this message after the specified amount of messages have been occurred.
                void BlockAfter(std::size_t occurrences);

                /**
                \brief Returns a report string for this message.
                \param[in] reportTypeName Specifies the name of the report type (e.g. "ERROR").
                \return Constructed report string containing all information of this message.
                */
                std::string ToReportString(const std::string& reportTypeName) const;

                //! Returns the message text.
                inline const std::string& GetText() const
                {
                    return text_;
                }

                //! Returns the source function where this message occurred.
                inline const std::string& GetSource() const
                {
                    return source_;
                }

                //! Returns the debug group name where this message occured.
                inline const std::string& GetGroupName() const
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

                std::string text_;
                std::string source_;
                std::string groupName_;
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
                LLGL::Log::PostReport(
                    LLGL::Log::ReportType::Error,
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

        std::map<std::string, Message>  errors_;
        std::map<std::string, Message>  warnings_;
        const char*                     source_     = "";
        const char*                     groupName_  = "";

};


} // /namespace LLGL


#endif



// ================================================================================
