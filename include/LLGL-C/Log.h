/*
 * Log.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_LOG_H
#define LLGL_C99_LOG_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>


/* ----- Enumerations ----- */

typedef enum LLGLReportType
{
    LLGLReportTypeDefault = 0,
    LLGLReportTypeError,
}
LLGLReportType;


/* ----- Types ----- */

typedef void* LLGLLogHandle;

typedef void (*LLGL_PFN_ReportCallback)(LLGLReportType type, const char* text, void* userData);


/* ----- Functions ----- */

LLGL_C_EXPORT void llglLogPrintf(const char* format, ...);
LLGL_C_EXPORT void llglLogErrorf(const char* format, ...);
LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallback(LLGL_PFN_ReportCallback callback, void* LLGL_NULLABLE(userData));
LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallbackReport(LLGLReport report);
LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallbackStd();
LLGL_C_EXPORT void llglUnregisterLogCallback(LLGLLogHandle handle);


#endif



// ================================================================================
