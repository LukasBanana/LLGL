/*
 * MacOSDebug.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 * Contributors
 * Lukas Hermanns (original author)
 * Kenneth Treadaway (Extended this Debug suite)
 *
 * Notes
 * KT I do not own a Mac so this is the best I can do from where I am sitting would appreciate it if  any kinks are worked out on the platform. I used apple website and opensource github to rough
 * what you see out so no guarantee of working yet. 
 */

#include "../Debug.h"
#include <stdio.h>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <sstream>
#include <ctime>
#import <Foundation/Foundation.h>
#import <AvailabilityMacros.h>
/**
 * @file AvailabilityMacros.h
 * @brief Defines platform and version availability macros for cross-platform development
 *
 * This header provides a set of preprocessor macros that help manage API availability
 * across different platforms, operating system versions, and SDK versions. These macros
 * are crucial for:
 * - Conditionally compiling code based on platform support
 * - Marking deprecated APIs
 * - Handling platform-specific feature availability
 *
 * Key Macro Categories:
 * 1. Platform Availability Macros
 *    - Define which platforms support specific APIs or features
 *    - Help manage conditional compilation for different OS versions
 *
 * 2. Deprecation Macros
 *    - Mark APIs that are no longer recommended
 *    - Provide compiler warnings for usage of deprecated functions
 *
 * 3. Availability Annotations
 *    - Specify when an API was introduced
 *    - Indicate the minimum OS version required for a particular feature
 *
 * Usage Examples:
 * @code
 * // Conditionally compile code for specific platform versions
 * #if defined(MAC_OS_X_VERSION_10_12) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12)
 *     // Code specific to macOS Sierra and later
 * #endif
 *
 * // Check API availability
 * #if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
 *     // Use newer API only if minimum deployment target supports it
 * #endif
 * @endcode
 *
 * Typical Macro Definitions:
 * - Platform version macros (e.g., MAC_OS_X_VERSION_10_12)
 * - Availability and deprecation annotations
 * - Conditional compilation directives
 *
 * @note This header is typically platform-specific and may vary between 
 *       different operating systems and development environments.
 *
 * @warning Incorrect usage of these macros can lead to compilation issues
 *          or unexpected runtime behavior.
 *
 * @see https://github.com/apple-oss-distributions/xnu/blob/xnu-11215.61.5/EXTERNAL_HEADERS/AvailabilityMacros.h
 */


namespace LLGL
{

namespace
{
    // Thread-local variable to track if this thread is already logging
    thread_local bool g_isLoggingThread = false;
    std::atomic<std::thread::id> g_currentOwner{};
    std::mutex g_debugMutex;

    struct DebugConfig
    {
        DebugLevel minLevel = DebugLevel::Info;
        DebugThreadMode threadMode = DebugThreadMode::Unsafe;
        DebugFormatFlags formatFlags = DebugFormatFlags::Default;
        bool toolsEnabled = false;
        char dateFormat[64] = "%Y-%m-%d %H:%M:%S";
    };
    
    DebugConfig g_config;
    
    /**
     * \brief Optimized lock acquisition for debug output.
     */
    std::unique_lock<std::mutex> GetOptimizedLock()
    {
        // Fast path - check if we're already the logging thread
        if (g_isLoggingThread)
            return std::unique_lock<std::mutex>();
            
        // Determine if we need a lock
        if (g_config.threadMode == DebugThreadMode::Unsafe)
            return std::unique_lock<std::mutex>();
            
        // Try to acquire lock without blocking first
        auto lock = std::unique_lock<std::mutex>(g_debugMutex, std::try_to_lock);
        if (!lock)
        {
            // If lock failed, take slow path with blocking acquisition
            lock = std::unique_lock<std::mutex>(g_debugMutex);
        }
        
        // Mark this thread as the logging thread
        g_isLoggingThread = true;
        g_currentOwner.store(std::this_thread::get_id());
        
        return lock;
    }
    
    /**
     * \brief RAII helper to reset thread logging state.
     */
    class LoggingThreadGuard
    {
        public:
            ~LoggingThreadGuard()
            {
                if (g_currentOwner.load() == std::this_thread::get_id())
                    g_isLoggingThread = false;
            }
    };
    
    /**
     * \brief Safely sets an environment variable with error checking.
     */
    bool SetEnvSafe(const char* name, const char* value, int overwrite)
    {
        if (::setenv(name, value, overwrite) != 0)
        {
            #ifdef LLGL_DEBUG
            // Log directly to stderr to avoid recursive calls
            ::fprintf(stderr, "LLGL Debug: Failed to set environment variable %s\n", name);
            #endif
            return false;
        }
        return true;
    }
    
    /**
     * \brief Checks if this macOS version supports memory debugging tools.
     */
    bool IsMemoryToolsSupported()
    {
        // Malloc debugging features available since OS X 10.6 Snow Leopard
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
            return true;
        #else
            // For older versions, check at runtime
            NSOperatingSystemVersion osVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
            return (osVersion.majorVersion > 10 || 
                   (osVersion.majorVersion == 10 && osVersion.minorVersion >= 6));
        #endif
    }

    /**
     * \brief Gets level indicator for debug output.
     */
    NSString* GetLevelIndicator(const DebugLevel level)
    {
        switch (level)
        {
            case DebugLevel::Trace:    return @"🔬";
            case DebugLevel::Verbose:  return @"🔍";
            case DebugLevel::Debug:    return @"🔧";
            case DebugLevel::Notice:   return @"📝";
            case DebugLevel::Info:     return @"ℹ️";
            case DebugLevel::Warning:  return @"⚠️";
            case DebugLevel::Error:    return @"❌";
            case DebugLevel::Fatal:    return @"💥";
            case DebugLevel::Silent:   return @"";
            default:                   return @"  ";
        }
    }
    
    /**
     * \brief Gets level name string.
     */
    const char* GetLevelName(const DebugLevel level)
    {
        switch (level)
        {
            case DebugLevel::Trace:    return "Trace";
            case DebugLevel::Verbose:  return "Verbose";
            case DebugLevel::Debug:    return "Debug";
            case DebugLevel::Notice:   return "Notice";
            case DebugLevel::Info:     return "Info";
            case DebugLevel::Warning:  return "Warning";
            case DebugLevel::Error:    return "Error";
            case DebugLevel::Fatal:    return "Fatal";
            case DebugLevel::Silent:   return "Silent";
            default:                   return "Unknown";
        }
    }
    
    /**
     * \brief Gets current thread identifier string.
     */
    std::string GetThreadId()
    {
        if (g_config.threadMode == DebugThreadMode::Identify)
        {
            std::thread::id threadId = std::this_thread::get_id();
            std::stringstream ss;
            ss << threadId;
            return "[Thread:" + ss.str() + "] ";
        }
        return "";
    }
    
    /**
     * \brief Gets formatted timestamp based on the current configuration.
     */
    std::string GetTimestamp()
    {
        if ((g_config.formatFlags & DebugFormatFlags::IncludeTimestamp) == 0)
            return "";
            
        std::time_t now = std::time(nullptr);
        char timestamp[128];
        std::strftime(timestamp, sizeof(timestamp), g_config.dateFormat, std::localtime(&now));
        return std::string("[") + timestamp + "] ";
    }
    
    /**
     * \brief Gets function context based on the current configuration.
     */
    std::string GetFunctionContext(const char* function)
    {
        if (!function || (g_config.formatFlags & DebugFormatFlags::IncludeFunction) == 0)
            return "";
            
        return std::string("[") + function + "] ";
    }
    
    /**
     * \brief Gets file and line context based on the current configuration.
     */
    std::string GetFileLineContext(const char* file, int line)
    {
        std::string result;
        
        if (file && (g_config.formatFlags & DebugFormatFlags::IncludeFile))
        {
            // Extract filename only, not full path
            const char* filename = file;
            const char* lastSlash = std::strrchr(file, '/');
            if (lastSlash)
                filename = lastSlash + 1;
                
            result += std::string("[") + filename;
            
            if (line > 0 && (g_config.formatFlags & DebugFormatFlags::IncludeLine))
                result += ":" + std::to_string(line);
                
            result += "] ";
        }
        else if (line > 0 && (g_config.formatFlags & DebugFormatFlags::IncludeLine))
        {
            result += "[line:" + std::to_string(line) + "] ";
        }
        
        return result;
    }
    
    /**
     * \brief Outputs debug message with specified level.
     */
    void DebugOutputWithLevel(const char* text, DebugLevel level, const char* function = nullptr, const char* file = nullptr, int line = 0)
    {
        // Skip if below minimum level or silent
        if (level < g_config.minLevel || level == DebugLevel::Silent)
            return;
            
        // Use optimized locking strategy
        auto lock = GetOptimizedLock();
        LoggingThreadGuard guard;
            
        #ifdef LLGL_DEBUG
        @autoreleasepool
        {
            /* Format message with all requested information */
            std::string timestamp = GetTimestamp();
            std::string threadId = GetThreadId();
            std::string functionContext = GetFunctionContext(function);
            std::string fileLineContext = GetFileLineContext(file, line);
            
            // Build complete message
            NSString* levelMsg = nil;
            if (text)
            {
                levelMsg = [NSString stringWithFormat:@"%@ %s%s%s%s%s", 
                               GetLevelIndicator(level),
                               timestamp.c_str(),
                               threadId.c_str(),
                               functionContext.c_str(),
                               fileLineContext.c_str(),
                               text];
            }
            else
            {
                levelMsg = [NSString stringWithFormat:@"%@ %s%s%s%s(null)", 
                               GetLevelIndicator(level),
                               timestamp.c_str(),
                               threadId.c_str(),
                               functionContext.c_str(),
                               fileLineContext.c_str()];
            }
                           
            NSLog(@"%@", levelMsg);
            
            /* Break into debugger for fatal errors */
            if (level == DebugLevel::Fatal && isatty(STDERR_FILENO))
            {
                LLGL_DEBUG_BREAK();
            }
        }
        #else
        /* Print to standard error stream with minimal formatting */
        std::string timestamp = GetTimestamp();
        std::string threadId = GetThreadId();
        const char* levelName = (level > DebugLevel::Info) ? GetLevelName(level) : "";
        
        ::fprintf(stderr, "%s%s%s%s\n", 
                 timestamp.c_str(),
                 levelName[0] ? "[" : "",
                 levelName,
                 levelName[0] ? "] " : "",
                 threadId.c_str(), 
                 text ? text : "(null)");
        #endif
    }
}

// Original debug function (maintained for compatibility)
LLGL_EXPORT void DebugPuts(const char* text)
{
    #ifdef LLGL_DEBUG
    /* Print to Xcode debug console */
    NSLog(@"%@\n", @(text));
    #else
    /* Print to standard error stream */
    ::fprintf(stderr, "%s\n", text);
    #endif
}

// Enhanced debug functions
LLGL_EXPORT void DebugTrace(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Trace, function, file, line);
}

LLGL_EXPORT void DebugVerbose(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Verbose, function, file, line);
}

LLGL_EXPORT void DebugOutput(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Debug, function, file, line);
}

LLGL_EXPORT void DebugNotice(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Notice, function, file, line);
}

LLGL_EXPORT void DebugInfo(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Info, function, file, line);
}

LLGL_EXPORT void DebugWarning(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Warning, function, file, line);
}

LLGL_EXPORT void DebugError(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Error, function, file, line);
}

LLGL_EXPORT void DebugFatal(const char* text, const char* function, const char* file, int line)
{
    DebugOutputWithLevel(text, DebugLevel::Fatal, function, file, line);
}

LLGL_EXPORT bool EnableDebugMemoryTools()
{
    #ifdef LLGL_DEBUG
    if (!g_config.toolsEnabled)
    {
        /* Check if memory tools are supported on this macOS version */
        if (!IsMemoryToolsSupported())
        {
            // Log directly to avoid potential recursion
            ::fprintf(stderr, "LLGL Debug: Memory debugging tools not supported on this macOS version\n");
            return false;
        }
    
        /* Enable Malloc Stack logging for memory debugger */
        if (!SetEnvSafe("MallocStackLogging", "1", 1))
            return false;
        
        /* Enable Guard Pages for buffer overflow detection */
        if (!SetEnvSafe("MallocGuardEdges", "1", 1))
            return false;
        
        /* Enable scribbling to detect use-after-free */
        if (!SetEnvSafe("MallocScribble", "1", 1))
            return false;
        
        g_config.toolsEnabled = true;
        return true;
    }
    #endif
    return false;
}

LLGL_EXPORT void DisableDebugMemoryTools()
{
    #ifdef LLGL_DEBUG
    if (g_config.toolsEnabled)
    {
        /* Disable previously enabled memory tracking */
        SetEnvSafe("MallocStackLogging", "0", 1);
        SetEnvSafe("MallocGuardEdges", "0", 1);
        SetEnvSafe("MallocScribble", "0", 1);
        g_config.toolsEnabled = false;
    }
    #endif
}

LLGL_EXPORT void SetDebugLevel(DebugLevel level)
{
    auto lock = GetOptimizedLock();
    LoggingThreadGuard guard;
    g_config.minLevel = level;
}

LLGL_EXPORT void SetDebugThreadMode(DebugThreadMode mode)
{
    // Need to lock even when switching to unsafe mode
    std::lock_guard<std::mutex> guard(g_debugMutex);
    g_config.threadMode = mode;
}

LLGL_EXPORT void SetDebugFormatFlags(DebugFormatFlags flags)
{
    auto lock = GetOptimizedLock();
    LoggingThreadGuard guard;
    g_config.formatFlags = flags;
}

LLGL_EXPORT void SetDebugDateFormat(const char* format)
{
    if (!format)
        return;
        
    auto lock = GetOptimizedLock();
    LoggingThreadGuard guard;
    ::strncpy(g_config.dateFormat, format, sizeof(g_config.dateFormat) - 1);
    g_config.dateFormat[sizeof(g_config.dateFormat) - 1] = '\0';
}

LLGL_EXPORT void LoadDebugConfigFromEnvironment()
{
    #ifdef LLGL_DEBUG
    const char* levelStr = ::getenv("LLGL_DEBUG_LEVEL");
    if (levelStr)
    {
        if (::strcasecmp(levelStr, "trace") == 0)
            SetDebugLevel(DebugLevel::Trace);
        else if (::strcasecmp(levelStr, "verbose") == 0)
            SetDebugLevel(DebugLevel::Verbose);
        else if (::strcasecmp(levelStr, "debug") == 0)
            SetDebugLevel(DebugLevel::Debug);
        else if (::strcasecmp(levelStr, "notice") == 0)
            SetDebugLevel(DebugLevel::Notice);
        else if (::strcasecmp(levelStr, "info") == 0)
            SetDebugLevel(DebugLevel::Info);
        else if (::strcasecmp(levelStr, "warning") == 0)
            SetDebugLevel(DebugLevel::Warning);
        else if (::strcasecmp(levelStr, "error") == 0)
            SetDebugLevel(DebugLevel::Error);
        else if (::strcasecmp(levelStr, "fatal") == 0)
            SetDebugLevel(DebugLevel::Fatal);
        else if (::strcasecmp(levelStr, "silent") == 0)
            SetDebugLevel(DebugLevel::Silent);
    }
    
    const char* memoryStr = ::getenv("LLGL_DEBUG_MEMORY");
    if (memoryStr && ::atoi(memoryStr) == 1)
    {
        EnableDebugMemoryTools();
    }
    
    const char* threadModeStr = ::getenv("LLGL_DEBUG_THREAD_MODE");
    if (threadModeStr)
    {
        if (::strcasecmp(threadModeStr, "unsafe") == 0)
            SetDebugThreadMode(DebugThreadMode::Unsafe);
        else if (::strcasecmp(threadModeStr, "safe") == 0)
            SetDebugThreadMode(DebugThreadMode::Safe);
        else if (::strcasecmp(threadModeStr, "identify") == 0)
            SetDebugThreadMode(DebugThreadMode::Identify);
    }
    
    const char* formatStr = ::getenv("LLGL_DEBUG_FORMAT");
    if (formatStr)
    {
        int formatFlags = DebugFormatFlags::Default;
        if (::strstr(formatStr, "timestamp"))
            formatFlags |= DebugFormatFlags::IncludeTimestamp;
        if (::strstr(formatStr, "function"))
            formatFlags |= DebugFormatFlags::IncludeFunction;
        if (::strstr(formatStr, "file"))
            formatFlags |= DebugFormatFlags::IncludeFile;
        if (::strstr(formatStr, "line"))
            formatFlags |= DebugFormatFlags::IncludeLine;
            
        SetDebugFormatFlags(static_cast<DebugFormatFlags>(formatFlags));
    }
    
    const char* dateFormatStr = ::getenv("LLGL_DEBUG_DATE_FORMAT");
    if (dateFormatStr)
    {
        SetDebugDateFormat(dateFormatStr);
    }
    #endif
}
