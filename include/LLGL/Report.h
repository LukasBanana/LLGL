/*
 * Report.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_REPORT_H
#define LLGL_REPORT_H


#include <LLGL/Export.h>
#include <LLGL/Container/StringView.h>
#include <string>


namespace LLGL
{


/**
\brief Error and warning report class.
\remarks To report errors globally, use the Log interface.
\see PipelineState::GetReport
\see RenderSystem::GetReport
\see Shader::GetReport
\see Log::Printf
\see Log::Errorf
*/
class LLGL_EXPORT Report final
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

        //! Copy constructor.
        Report(const Report& rhs);

        //! Move constructor.
        Report(Report&& rhs);

        //! Deletes the internal report.
        ~Report();

    public:

        /**
        \brief Returns a NUL-terminated string of the report text. This must never be null.
        \remarks LLGL backends always append the newline character <tt>'\\n'</tt> at the end of formatted string.
        This is not required, but makes reports with either a single or multiple lines consistent.
        Therefore, printing such a report to the standard output does not require an additional newline character:
        \code
        // C++ standard output:
        std::cout << myReport.GetText();

        // C standard output:
        printf("%s", myReport.GetText());

        // LLGL Log output:
        LLGL::Log::Printf("%s", myReport.GetText());
        \endcode
        */
        const char* GetText() const;

        //! Returns true if this report contains error messages.
        bool HasErrors() const;

        //! Overrides the report with a copy of the specified text and flag.
        void Reset(const char* text, bool hasErrors);

        //! Overrides the report with a copy of the specified text and flag.
        void Reset(const StringView& text, bool hasErrors);

        //! Overrides the report by taking the ownership of the specified string.
        void Reset(std::string&& text, bool hasErrors);

        /**
        \brief Appends a formatted message to this report. The previous error flag remains unchanged.
        \param[in] format Specifies the formatted message. Same as \c ::printf.
        */
        void Printf(const char* format, ...);

        /**
        \brief Appends a formatted message to this report and sets the error flag to \c true.
        \param[in] format Specifies the formatted message. Same as \c ::printf.
        */
        void Errorf(const char* format, ...);

    public:

        //! Copy operator.
        Report& operator = (const Report& rhs);

        //! Move operator.
        Report& operator = (Report&& rhs);

        //! Returns true if this report has a non-empty text or is marked as having errors.
        operator bool () const;

    private:

        struct Pimpl;
        Pimpl* pimpl_;

};


} // /namespace LLGL


#endif



// ================================================================================
