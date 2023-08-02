/*
 * Report.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_REPORT_H
#define LLGL_C99_REPORT_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>
#include <stdbool.h>


LLGL_C_EXPORT const char* llglGetReportText(LLGLReport report);
LLGL_C_EXPORT bool llglHasReportErrors(LLGLReport report);


#endif



// ================================================================================
