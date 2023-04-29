/*
 * Report.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_REPORT_H
#define LLGL_REPORT_H


#include <LLGL/NonCopyable.h>
#include <LLGL/Container/StringView.h>
#include <string>


namespace LLGL
{


/**
\brief Error and warning report interface.
\see PipelineState::GetReport
*/
class LLGL_EXPORT Report : public NonCopyable
{

    public:

        //! Constructs an empty report.
        Report();

        //! Constructs the report with the specified text and flag.
        Report(const char* text, bool hasErrors);

        //! Constructs the report with the specified text and flag.
        Report(const StringView& text, bool hasErrors);

        //! Constructs the report by taking the ownersip of the specified string.
        Report(std::string&& text, bool hasErrors);

        //! Move constructor.
        Report(Report&& rhs);

        //! Move operator.
        Report& operator = (Report&& rhs);

        //! Deletes the internal report.
        ~Report();

    public:

        //! Returns a NUL-terminated string of the report text. This must never be null.
        const char* GetText() const;

        //! Returns true if this report contains error messages.
        bool HasErrors() const;

        //! Overrides the report with a copy of the specified text and flag.
        void Reset(const char* text, bool hasErrors);

        //! Overrides the report with a copy of the specified text and flag.
        void Reset(const StringView& text, bool hasErrors);

        //! Overrides the report by taking the ownership of the specified string.
        void Reset(std::string&& text, bool hasErrors);

    public:

        //! Returns true if this report has a non-empty text or is marked as having errors.
        operator bool () const;

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
