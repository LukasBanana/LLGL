/*
 * Log.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Log.h>
#include <mutex>


namespace LLGL
{

namespace Log
{


struct LogState
{
    std::mutex      reportMutex;
    ReportCallback  reportCallback  = nullptr;
    void*           userData        = nullptr;
};

static LogState g_logState;


/* ----- Functions ----- */

LLGL_EXPORT void PostReport(ReportType type, const std::string& message, const std::string& contextInfo)
{
    ReportCallback  callback;
    void*           userData = nullptr;

    /* Get callback and user data with a lock guard */
    {
        std::lock_guard<std::mutex> guard { g_logState.reportMutex };
        callback = g_logState.reportCallback;
        userData = g_logState.userData;
    }

    /* Post report to callback */
    if (callback != nullptr)
        callback(type, message, contextInfo, userData);
}

LLGL_EXPORT void SetReportCallback(const ReportCallback& callback, void* userData)
{
    std::lock_guard<std::mutex> guard { g_logState.reportMutex };
    g_logState.reportCallback   = callback;
    g_logState.userData         = userData;
}


} // /namespace Log

} // /namespace LLGL



// ================================================================================
