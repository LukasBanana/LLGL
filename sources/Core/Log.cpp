/*
 * Log.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Log.h>
#include <LLGL/Platform/Platform.h>
#include "../Platform/ConsoleManip.h"
#include "CoreUtils.h"
#include "StringUtils.h"
#include "../Renderer/ContainerTypes.h"
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <stdarg.h>

#ifdef LLGL_OS_ANDROID
#   include <android/log.h>
#endif


namespace LLGL
{

namespace Log
{


struct LogListener
{
    LogListener() = default;
    LogListener(const LogListener&) = default;
    LogListener& operator = (const LogListener&) = default;

    inline LogListener(const ReportCallback& callback, void* userData = nullptr) :
        callback { callback },
        userData { userData }
    {
    }

    inline LogListener(const ReportCallbackExt& callback, void* userData = nullptr) :
        callbackExt { callback },
        userData    { userData }
    {
    }

    ReportCallback      callback    = nullptr;
    ReportCallbackExt   callbackExt = nullptr;
    void*               userData    = nullptr;

    inline void Invoke(ReportType type, const char* text, const ColorCodes& colors)
    {
        if (callback != nullptr)
            callback(type, text, userData);
        else if (callbackExt != nullptr)
            callbackExt(type, text, userData, colors);
    }
};

using LogListenerPtr = std::unique_ptr<LogListener>;

struct LogState
{
    std::mutex                              lock;
    UnorderedUniquePtrVector<LogListener>   listeners;
    LogListenerPtr                          listenerStd;
};

class TrivialLock
{

    public:

        inline void lock()
        {
            value_ = true;
        }

        void unlock()
        {
            value_ = false;
        }

        inline operator bool () const
        {
            return value_;
        }

    private:

        bool value_ = false;

};

static LogState                 g_logState;
static thread_local TrivialLock g_logRecursionLock;


/* ----- Functions ----- */

static void PostReport(ReportType type, const char* text, const ColorCodes& colors = {})
{
    std::lock_guard<std::mutex> guard{ g_logState.lock };

    if (LogListener* listenerStd = g_logState.listenerStd.get())
        listenerStd->Invoke(type, text, colors);

    for (const auto& listener : g_logState.listeners)
        listener->Invoke(type, text, colors);
}

LLGL_EXPORT void Printf(const char* format, ...)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<TrivialLock> guard{ g_logRecursionLock };
        std::string str;
        LLGL_STRING_PRINTF(str, format);
        PostReport(ReportType::Default, str.c_str());
    }
}

LLGL_EXPORT void Printf(const ColorCodes& colors, const char* format, ...)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<TrivialLock> guard{ g_logRecursionLock };
        std::string str;
        LLGL_STRING_PRINTF(str, format);
        PostReport(ReportType::Default, str.c_str(), colors);
    }
}

LLGL_EXPORT void Errorf(const char* format, ...)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<TrivialLock> guard{ g_logRecursionLock };
        std::string str;
        LLGL_STRING_PRINTF(str, format);
        PostReport(ReportType::Error, str.c_str());
    }
}

LLGL_EXPORT void Errorf(const ColorCodes& colors, const char* format, ...)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<TrivialLock> guard{ g_logRecursionLock };
        std::string str;
        LLGL_STRING_PRINTF(str, format);
        PostReport(ReportType::Error, str.c_str(), colors);
    }
}

template <typename TCallback>
static LogHandle RegisterCallbackInternal(const TCallback& callback, void* userData)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<std::mutex> guard{ g_logState.lock };
        return reinterpret_cast<LogHandle>(g_logState.listeners.emplace<LogListener>(callback, userData));
    }
    return nullptr;
}

LLGL_EXPORT LogHandle RegisterCallback(const ReportCallback& callback, void* userData)
{
    return RegisterCallbackInternal<ReportCallback>(callback, userData);
}

LLGL_EXPORT LogHandle RegisterCallback(const ReportCallbackExt& callback, void* userData)
{
    return RegisterCallbackInternal<ReportCallbackExt>(callback, userData);
}

LLGL_EXPORT LogHandle RegisterCallbackReport(Report& report)
{
    return RegisterCallback(
        [](ReportType type, const char* text, void* userData)
        {
            if (auto* report = reinterpret_cast<Report*>(userData))
            {
                if (type == ReportType::Error)
                    report->Errorf("%s", text);
                else
                    report->Printf("%s", text);
            }
        },
        &report
    );
}

static void PrintToStandardOutput(ReportType type, const char* text)
{
    #ifdef LLGL_OS_ANDROID
    (void)__android_log_print((type == ReportType::Error ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO), "LLGL", "%s", text);
    #else
    ::fprintf((type == ReportType::Error ? stderr : stdout), "%s", text);
    #endif
}

static void StandardOutputReportCallback(ReportType type, const char* text, void* /*userData*/)
{
    /* Print text to standard output without console state changes */
    PrintToStandardOutput(type, text);
}

static void StandardOutputReportCallbackExt(ReportType type, const char* text, void* /*userData*/, const ColorCodes& colors)
{
    if (colors.textFlags != 0 || colors.backgroundFlags != 0)
    {
        /* Print text to standard output with temporarily changing colors */
        ConsoleManip::ScopedConsoleColors scopedColors{ type, colors };
        PrintToStandardOutput(type, text);
    }
    else
    {
        /* Print text to standard output without console state changes */
        PrintToStandardOutput(type, text);
    }
}

LLGL_EXPORT LogHandle RegisterCallbackStd(long stdOutFlags)
{
    if (!g_logRecursionLock)
    {
        std::lock_guard<std::mutex> guard{ g_logState.lock };
        if (g_logState.listenerStd.get() == nullptr)
        {
            if ((stdOutFlags & StdOutFlags::Colored) != 0)
                g_logState.listenerStd = MakeUnique<LogListener>(StandardOutputReportCallbackExt);
            else
                g_logState.listenerStd = MakeUnique<LogListener>(StandardOutputReportCallback);
        }
        return reinterpret_cast<LogHandle>(g_logState.listenerStd.get());
    }
    return LogHandle{};
}

LLGL_EXPORT void UnregisterCallback(LogHandle handle)
{
    if (handle != nullptr)
    {
        std::lock_guard<std::mutex> guard{ g_logState.lock };
        if (handle == reinterpret_cast<LogHandle>(g_logState.listenerStd.get()))
            g_logState.listenerStd.reset();
        else
            g_logState.listeners.erase(reinterpret_cast<LogListener*>(handle));
    }
}


} // /namespace Log

} // /namespace LLGL



// ================================================================================
