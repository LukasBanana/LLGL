/*
 * MacOSDebug.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_DEBUG_H
#define LLGL_MACOS_DEBUG_H

#include <signal.h>

// Original debug break functionality
#ifdef SIGTRAP
#   define LLGL_DEBUG_BREAK() raise(SIGTRAP)
#else
#   define LLGL_DEBUG_BREAK()
#endif

// Convenience macros for debug functions with automatic function context
#ifdef LLGL_DEBUG
#   define LLGL_DEBUG_TRACE(msg) LLGL::DebugTrace(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_VERBOSE(msg) LLGL::DebugVerbose(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_OUTPUT(msg) LLGL::DebugOutput(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_NOTICE(msg) LLGL::DebugNotice(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_INFO(msg) LLGL::DebugInfo(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_WARNING(msg) LLGL::DebugWarning(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_ERROR(msg) LLGL::DebugError(msg, __FUNCTION__, __FILE__, __LINE__)
#   define LLGL_DEBUG_FATAL(msg) LLGL::DebugFatal(msg, __FUNCTION__, __FILE__, __LINE__)
#else
#   define LLGL_DEBUG_TRACE(msg)
#   define LLGL_DEBUG_VERBOSE(msg)
#   define LLGL_DEBUG_OUTPUT(msg)
#   define LLGL_DEBUG_NOTICE(msg)
#   define LLGL_DEBUG_INFO(msg)
#   define LLGL_DEBUG_WARNING(msg)
#   define LLGL_DEBUG_ERROR(msg)
#   define LLGL_DEBUG_FATAL(msg)
#endif

namespace LLGL
{

/**
 * \brief Debug message severity levels.
 */
enum class DebugLevel
{
    Trace,      //!< Most detailed level for function entry/exit tracing
    Verbose,    //!< Detailed information for granular debugging
    Debug,      //!< Standard debug information
    Notice,     //!< Noteworthy events that aren't warnings
    Info,       //!< Information message for general debugging
    Warning,    //!< Warning message for potential issues
    Error,      //!< Error message for recoverable problems
    Fatal,      //!< Fatal error that should terminate the application
    Silent      //!< No output (for completely disabling logging)
};

/**
 * \brief Thread safety mode for debugging operations.
 */
enum class DebugThreadMode
{
    Unsafe,     //!< No thread safety (fastest)
    Safe,       //!< Thread-safe debug operations
    Identify    //!< Thread-safe and includes thread ID in messages
};

/**
 * \brief Format flags for debug message output.
 */
enum DebugFormatFlags
{
    Default           = 0,
    IncludeTimestamp  = (1 << 0),  //!< Include timestamp in messages
    IncludeFunction   = (1 << 1),  //!< Include function name in messages
    IncludeFile       = (1 << 2),  //!< Include source file in messages
    IncludeLine       = (1 << 3),  //!< Include line number in messages
    Full              = IncludeTimestamp | IncludeFunction | IncludeFile | IncludeLine
};

/**
 * \brief Outputs the specified text message to the debug console.
 * \param[in] text Specifies the message to output.
 * 
 * \remarks This is the original debug output function for backward compatibility.
 * 
 * \note The newer debug functions (DebugInfo, DebugWarning, etc.) provide more features.
 */
LLGL_EXPORT void DebugPuts(const char* text);

/**
 * \brief Outputs a trace message to the debug console.
 * \param[in] text Specifies the trace message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \remarks Trace level is the most detailed level, intended for function entry/exit tracing.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_TRACE("Entering render loop");
 * // Or direct function call:
 * LLGL::DebugTrace("Entering render loop", "RenderSystem::Render");
 * \endcode
 */
LLGL_EXPORT void DebugTrace(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs a verbose message to the debug console.
 * \param[in] text Specifies the verbose message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \remarks Verbose messages are for detailed debugging information.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_VERBOSE("Processing vertex buffer with 1024 vertices");
 * \endcode
 */
LLGL_EXPORT void DebugVerbose(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs a debug message to the debug console.
 * \param[in] text Specifies the debug message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_OUTPUT("Created texture with format RGB8");
 * \endcode
 */
LLGL_EXPORT void DebugOutput(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs a notice message to the debug console.
 * \param[in] text Specifies the notice message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \remarks Notice messages highlight significant events that aren't warnings.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_NOTICE("Switching to fallback shader");
 * \endcode
 */
LLGL_EXPORT void DebugNotice(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs an info message to the debug console.
 * \param[in] text Specifies the info message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_INFO("Initializing renderer");
 * \endcode
 */
LLGL_EXPORT void DebugInfo(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs a warning message to the debug console.
 * \param[in] text Specifies the warning message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_WARNING("Deprecated shader feature used");
 * \endcode
 */
LLGL_EXPORT void DebugWarning(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs an error message to the debug console.
 * \param[in] text Specifies the error message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_ERROR("Failed to create texture");
 * \endcode
 */
LLGL_EXPORT void DebugError(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Outputs a fatal error message to the debug console and breaks into debugger if attached.
 * \param[in] text Specifies the fatal error message to output.
 * \param[in] function Optional function name for context.
 * \param[in] file Optional source file name.
 * \param[in] line Optional line number.
 * 
 * \remarks Fatal errors will trigger a debugger breakpoint if a debugger is attached.
 * 
 * \code
 * // Example usage:
 * LLGL_DEBUG_FATAL("Critical initialization failure");
 * \endcode
 */
LLGL_EXPORT void DebugFatal(const char* text, const char* function = nullptr, const char* file = nullptr, int line = 0);

/**
 * \brief Enables memory debugging tools in Xcode.
 * 
 * \return True if memory tools were successfully enabled, false if not supported.
 * 
 * \remarks This enables:
 *          - MallocStackLogging: Records allocation call stacks
 *          - MallocGuardEdges: Adds guard pages for buffer overflow detection
 *          - MallocScribble: Helps detect use-after-free bugs
 * 
 * \note Memory tools are disabled by default and only available in debug builds.
 * 
 * \code
 * // Example usage:
 * if (LLGL::EnableDebugMemoryTools()) {
 *     std::cout << "Memory debugging enabled" << std::endl;
 * }
 * \endcode
 */
LLGL_EXPORT bool EnableDebugMemoryTools();

/**
 * \brief Disables previously enabled memory debugging tools.
 * 
 * \code
 * // Example usage:
 * LLGL::EnableDebugMemoryTools();
 * // ... debug memory-intensive code ...
 * LLGL::DisableDebugMemoryTools();
 * \endcode
 */
LLGL_EXPORT void DisableDebugMemoryTools();

/**
 * \brief Sets minimum debug level for output messages.
 * \param[in] level Minimum level to output.
 * 
 * \remarks Messages with severity below this level will be suppressed.
 * 
 * \code
 * // Example usage:
 * // Only show warnings and more severe messages
 * LLGL::SetDebugLevel(LLGL::DebugLevel::Warning);
 * \endcode
 */
LLGL_EXPORT void SetDebugLevel(DebugLevel level);

/**
 * \brief Sets thread safety mode for debug operations.
 * \param[in] mode Thread safety mode to use.
 * 
 * \remarks Different thread modes offer trade-offs between performance and thread safety:
 *          - Unsafe: No thread safety, fastest but may cause interleaved output
 *          - Safe: Thread-safe debug operations
 *          - Identify: Thread-safe and includes thread ID in messages
 * 
 * \code
 * // Example usage for multi-threaded application:
 * LLGL::SetDebugThreadMode(LLGL::DebugThreadMode::Identify);
 * \endcode
 */
LLGL_EXPORT void SetDebugThreadMode(DebugThreadMode mode);

/**
 * \brief Sets format flags for debug message output.
 * \param[in] flags Format flags to use.
 * 
 * \see DebugFormatFlags
 * 
 * \code
 * // Example usage - include timestamps and function names:
 * LLGL::SetDebugFormatFlags(
 *     LLGL::DebugFormatFlags::IncludeTimestamp | 
 *     LLGL::DebugFormatFlags::IncludeFunction
 * );
 * 
 * // Use all available formatting:
 * LLGL::SetDebugFormatFlags(LLGL::DebugFormatFlags::Full);
 * \endcode
 */
LLGL_EXPORT void SetDebugFormatFlags(DebugFormatFlags flags);

/**
 * \brief Sets date format for the timestamp in debug messages.
 * \param[in] format C-style strftime format string.
 * 
 * \remarks Only applies if the IncludeTimestamp flag is set.
 * 
 * \code
 * // Example usage - show only time in 24-hour format:
 * LLGL::SetDebugDateFormat("%H:%M:%S");
 * 
 * // Example usage - show date and time:
 * LLGL::SetDebugDateFormat("%Y-%m-%d %H:%M:%S");
 * \endcode
 */
LLGL_EXPORT void SetDebugDateFormat(const char* format);

/**
 * \brief Loads debug configuration from environment variables.
 * 
 * \remarks Checks for:
 *          - LLGL_DEBUG_LEVEL: Minimum debug level (values: trace, verbose, debug, notice, info, warning, error, fatal, silent)
 *          - LLGL_DEBUG_MEMORY: Enable memory tools (values: 0, 1)
 *          - LLGL_DEBUG_THREAD_MODE: Thread safety (values: unsafe, safe, identify)
 *          - LLGL_DEBUG_FORMAT: Format flags (values: timestamp, function, file, line)
 *          - LLGL_DEBUG_DATE_FORMAT: Custom date format for timestamps
 * 
 * \code
 * // Example usage at application startup:
 * int main(int argc, char* argv[]) {
 *     LLGL::LoadDebugConfigFromEnvironment();
 *     // ... rest of application ...
 * }
 * \endcode
 * 
 * \note This allows configuration without recompiling by setting environment variables:
 * 
 * \code
 * # Example shell configuration:
 * export LLGL_DEBUG_LEVEL=warning
 * export LLGL_DEBUG_MEMORY=1
 * export LLGL_DEBUG_THREAD_MODE=identify
 * export LLGL_DEBUG_FORMAT="timestamp function"
 * export LLGL_DEBUG_DATE_FORMAT="%H:%M:%S"
 * \endcode
 */
LLGL_EXPORT void LoadDebugConfigFromEnvironment();

} // namespace LLGL

#endif // LLGL_MACOS_DEBUG_H
