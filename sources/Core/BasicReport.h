/*
 * BasicReport.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BASIC_REPORT_H
#define LLGL_BASIC_REPORT_H


#include <LLGL/Report.h>
#include <LLGL/Container/StringView.h>
#include <string>


namespace LLGL
{


/**
\brief Error and warning report interface.
\see PipelineState::GetReport
*/
class LLGL_EXPORT BasicReport final : public Report
{

    public:

        BasicReport() = default;

        // Initializes the report with text and error bit.
        BasicReport(const StringView& text, bool hasErrors = false);

        // Resets the report.
        void Reset(const StringView& text, bool hasErrors = false);
        void Reset(std::string&& text, bool hasErrors = false);

        const char* GetText() const override;
        bool HasErrors() const override;

    public:

        // Returns whether this is a valid report. Otherwise, there is no text and no errors.
        inline operator bool() const
        {
            return (!text_.empty() || hasErrors_);
        }

    private:

        std::string text_;
        bool        hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
