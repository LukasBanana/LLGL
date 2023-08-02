/*
 * C99Report.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Report.h>
#include <LLGL-C/Report.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT const char* llglGetReportText(LLGLReport report)
{
    if (const Report* internalReport = LLGL_PTR(Report, report))
        return internalReport->GetText();
    else
        return "";
}

LLGL_C_EXPORT bool llglHasReportErrors(LLGLReport report)
{
    if (const Report* internalReport = LLGL_PTR(Report, report))
        return internalReport->HasErrors();
    else
        return false;
}


// } /namespace LLGL



// ================================================================================
