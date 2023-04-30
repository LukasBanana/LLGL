/*
 * Log.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LOG_H
#define LLGL_LOG_H


#include <LLGL/Export.h>
#include <LLGL/Report.h>
#include <functional>


namespace LLGL
{

namespace Log
{


/* ----- Enumerations ----- */

/**
\brief Report type enumeration.
\see ReportCallback
*/
enum class ReportType
{
    /**
    \brief Default report type. Usually forwarded to \c stdout or \c std::cout.
    \see Printf
    */
    Default = 0,

    /**
    \brief Error message type. Usualyl forwarded to \c stderr \c std::cerr.
    \see Errorf.
    */
    Error,
};


/* ----- Types ----- */

/**
\brief Opaque handle to a log callback.
\remarks This is only used to unregister a previously registered log callback.
\see UnregisterCallback
*/
using LogHandle = void*;

/**
\brief Report callback function signature.
\param[in] type Specifies the type of the report message.
\param[in] text Pointer to a NUL-terminated string of the report text.
\param[in] userData Specifies the user data that was set in the previous call to SetReportCallback.
\see ReportType
\see SetReportCallback
*/
using ReportCallback = std::function<void(ReportType type, const char* text, void* userData)>;


/* ----- Functions ----- */

/**
\brief Prints a formatted message to the log.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Printf
*/
LLGL_EXPORT void Printf(const char* format, ...);

/**
\brief Prints a formatted error message to the log.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Errorf
*/
LLGL_EXPORT void Errorf(const char* format, ...);

/**
\brief Registers a new log callback. No log callback is specified by default, in which case the reports are ignored.
\param[in] callback Specifies the new report callback. This can also be null.
\param[in] userData Optional raw pointer to some user data that will be passed to the callback each time a report is generated.
\returns Opaque handle to this log callback or null if this is called recursively, i.e. inside another log callback function.
It can be used to unregister the callback.
\remarks The reports can be generated in a multi-threaded environment. Even this function can be called on multiple threads.
The functionality of the entire Log namespace is synchronized by LLGL.
Use RegisterCallbackStd to forward the reports to the standard C++ I/O streams.
\see RegisterCallbackStd
*/
LLGL_EXPORT LogHandle RegisterCallback(const ReportCallback& callback, void* userData = nullptr);

/**
\brief Registers a new log callback that is forwarded to the specified Report.
\param[out] report Specifies the report that will receive all log entries.
\remarks This function forwards all log entries to the Report::Printf and Report::Errorf functions respectively.
\returns Opaque handle to this log callback or null if this is called recursively, i.e. inside another log callback function.
It can be used to unregister the callback.
\see Report
*/
LLGL_EXPORT LogHandle RegisterCallbackReport(Report& report);

/**
\brief Registers a new log callback to the standard output streams, i.e. \c stdout and \c stderr from <tt><stdio.h></tt>.
\returns Opaque handle to this log callback or null if this is called recursively, i.e. inside another log callback function.
It can be used to unregister the callback.
\remarks If there already is a registered handle for the standard output,
this function only returns the previously registered handle that is associated with the standard output.
\see SetReportCallback
*/
LLGL_EXPORT LogHandle RegisterCallbackStd();

/**
\brief Unregisteres the specified handle from the log output.
\param[in] handle Specifies the opaque handle that was returned from a previous call to RegisterCallback, RegisterCallbackReport, or RegisterCallbackStd.
\see RegisterCallback
\see RegisterCallbackReport
\see RegisterCallbackStd
*/
LLGL_EXPORT void UnregisterCallback(LogHandle handle);


} // /namespace Log

} // /namespace LLGL


#endif



// ================================================================================
