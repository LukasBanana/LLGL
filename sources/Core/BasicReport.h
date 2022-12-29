/*
 * BasicReport.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

    private:

        std::string text_;
        bool        hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
