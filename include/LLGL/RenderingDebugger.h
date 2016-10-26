/*
 * RenderingDebugger.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
};

enum class WarningType
{
    ImproperArgument,   //!< Warning due to improper argument (e.g. generating 4 vertices while having triangle list as primitive topology).
    ImproperState,      //!< Warning due to improper state (e.g. rendering while viewport is not visible).
    PointlessOperation, //!< Warning due to a operation without any effect (e.g. drawing with 0 vertices).
};


/**
\brief Rendering debugger interface.
\remarks This can be used to profile the renderer draw calls and buffer updates.
*/
class LLGL_EXPORT RenderingDebugger
{

    public:

        virtual ~RenderingDebugger();
        
        //! Sets the new source function name.
        void SetSource(const char* source);

        /**
        \brief Posts an error message.
        \param[in] type Specifies the type of error.
        \param[in] message Specifies the string which describes the failure.
        \param[in] source Specifies the string which describes the source (typically the function where the failure happend).
        */
        void PostError(const ErrorType type, const std::string& message);

        /**
        \brief Posts a warning message.
        \param[in] type Specifies the type of error.
        \param[in] message Specifies the string which describes the warning.
        \param[in] source Specifies the string which describes the source (typically the function where the failure happend).
        */
        void PostWarning(const WarningType type, const std::string& message);

    protected:

        //! Rendering debugger message class.
        class LLGL_EXPORT Message
        {

            public:

                Message() = default;
                Message(const Message&) = default;
                Message& operator = (const Message&) = default;

                Message(const std::string& text, const std::string& source);

                //! Blocks further occurrences of this message.
                void Block();

                //! Blocks further occurrences of this message after the specified amount of messages have been occurred.
                void BlockAfter(std::size_t occurrences);

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
                std::size_t occurrences_    = 1;
                bool        blocked_        = false;

        };

        RenderingDebugger() = default;

        virtual void OnError(ErrorType type, Message& message);
        
        virtual void OnWarning(WarningType type, Message& message);

    private:

        std::map<std::string, Message>  errors_;
        std::map<std::string, Message>  warnings_;
        const char*                     source_     = "";

};


} // /namespace LLGL


#endif



// ================================================================================
