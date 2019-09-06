/*
 * Log.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LOG_H
#define LLGL_LOG_H


#include "Export.h"
#include <functional>
#include <string>
#include <iostream>


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
    \brief Error message type.
    \remarks For example, when a feature is used that is not supported.
    */
    Error,

    /**
    \brief Warning message type.
    \remarks For example, when an operation has no effect like submitting a draw command with zero vertices.
    */
    Warning,

    /**
    \brief Information message type.
    \remarks For example, when a multi-sampling format is not supported so it's set to a lower quality than it was specified.
    */
    Information,

    /**
    \brief Performance penelty message type.
    \remarks For example, when unnecessary clear commands are submitted.
    */
    Performance,
};


/* ----- Types ----- */

/**
\brief Report callback function signature.
\param[in] type Specifies the type of the report message.
\param[in] message Specifies the report message.
\param[in] contextInfo Specifies a descriptive string about the context of the report (e.g. <code>"in 'LLGL::RenderSystem::CreateShader'"</code>). This may also be empty.
\param[in] userData Specifies the user data that was set in the previous call to SetReportCallback.
\see ReportType
\see SetReportCallback
*/
using ReportCallback = std::function<void(ReportType type, const std::string& message, const std::string& contextInfo, void* userData)>;


/* ----- Functions ----- */

/**
\brief Posts a report to the currently set report callback.
\see ReportCallback
*/
LLGL_EXPORT void PostReport(ReportType type, const std::string& message, const std::string& contextInfo = "");

/**
\brief Sets the new report callback. No report callback is specified by default, in which case the reports are ignored.
\param[in] callback Specifies the new report callback. This can also be null.
\param[in] userData Optional raw pointer to some user data that will be passed to the callback each time a report is generated.
\remarks The reports can be generated in a multi-threaded environment. Even this function can be called on multiple threads.
The functionality of the entire Log namespace is synchronized by LLGL.
Use SetReportCallbackStd to forward the reports to the standard C++ I/O streams.
\see PostReport
\see SetReportCallbackStd
*/
LLGL_EXPORT void SetReportCallback(const ReportCallback& callback, void* userData = nullptr);

/**
\brief Sets the new report callback to the standard output streams.
\param[in] stream Specifies the output stream. By default <code>std::cerr</code>.
\see SetReportCallback
*/
LLGL_EXPORT void SetReportCallbackStd(std::ostream& stream = std::cerr);

/**
\brief Sets the maximum number of reports that will be triggered. All remaining reports will be ignored.
\param[in] maxCount Specifies the maximum number of reports. If this is 0, there is affectively no limit. By default 0.
*/
LLGL_EXPORT void SetReportLimit(std::size_t maxCount);


} // /namespace Log

} // /namespace LLGL


#endif



// ================================================================================
