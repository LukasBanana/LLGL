/*
 * ReportUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_REPORT_UTILS_H
#define LLGL_REPORT_UTILS_H


#include <LLGL/Report.h>


namespace LLGL
{


// Ensures the specified reports ends with a newline character as long as it's not empty.
inline void EndReportWithNewline(Report& report)
{
    char end = *report.GetText();
    if (end != '\0' && end != '\n')
        report.Printf("\n");
}

// Resets the specified report and ensures it ends with a newline character.
inline void ResetReportWithNewline(Report& report, std::string&& text, bool hasErrors)
{
    report.Reset(std::forward<std::string>(text), hasErrors);
    EndReportWithNewline(report);
}


} // /namespace LLGL


#endif



// ================================================================================
