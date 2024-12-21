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


 //! Encodes the flags for full RGB console colors.
#define LLGL_LOG_RGB(R, G, B)               \
    (                                       \
        LLGL::Log::ColorFlags::FullRGB |    \
        (((R) & 0xFF) << 24) |              \
        (((G) & 0xFF) << 16) |              \
        (((B) & 0xFF) <<  8)                \
    )


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
    \brief Error message type. Usually forwarded to \c stderr or \c std::cerr.
    \see Errorf.
    */
    Error,
};


/* ----- Flags ----- */

/**
\brief Standard output flags enumeration.
\see RegisterCallbackStd
*/
struct StdOutFlags
{
    enum
    {
        //! Enables color output. By default, no color is printed to the standard output.
        Colored = (1 << 0),
    };
};

/**
\brief Enumeration of all log color code flags.
\see ColorCodes::textFlags
\see ColorCodes::backgroundFlags
*/
struct ColorFlags
{
    enum
    {
        //! Resets the color codes to their default values.
        Default         = (1 << 0),

        //! Red color component in 4 bit color palette.
        Red             = (1 << 1),

        //! Green color component in 4 bit color palette.
        Green           = (1 << 2),

        //! Blue color component in 4 bit color palette.
        Blue            = (1 << 3),

        //! Makes the color brighter in 4 bit color palette.
        Bright          = (1 << 4),

        //! Applies bold/intensity to text for extended color palettes.
        Bold            = (1 << 5),

        //! Adds an underline to the text for extended color palettes.
        Underline       = (1 << 6),

        /**
        \brief Uses fully RGB encoded color.
        \remarks This should be used with the \c LLGL_LOG_RGB macro like in this example:
        \code
        // Print error with RGB color (red=200, green=50, blue=0)
        LLGL::Log::Printf(LLGL_LOG_RGB(200, 50, 0), "error");
        \encode
        */
        FullRGB         = (1 << 7),

        //! Combined flags for yellow color (red and green).
        Yellow          = (Red | Green),

        //! Combined flags for pink color (red and blue).
        Pink            = (Red | Blue),

        //! Combined flags for cyan color (green and blue).
        Cyan            = (Green | Blue),

        //! Combined flags for gray color.
        Gray            = (Red | Green | Blue),

        //! Combined flags for bright red color.
        BrightRed       = (Bright | Red),

        //! Combined flags for bright green color.
        BrightGreen     = (Bright | Green),

        //! Combined flags for bright blue color.
        BrightBlue      = (Bright | Blue),

        //! Combined flags for bright yellow color (red and green).
        BrightYellow    = (Bright | Yellow),

        //! Combined flags for bright pink color (red and blue).
        BrightPink      = (Bright | Pink),

        //! Combined flags for bright cyan color (green and blue).
        BrightCyan      = (Bright | Cyan),

        //! Combined flags for white color.
        White           = (Bright | Gray),

        //! Combined flags for standard error reports (bold and dark red).
        StdError        = (Bold | Red),

        //! Combined flags for standard warning reports (bold and bright yellow).
        StdWarning      = (Bold | BrightYellow),

        //! Combined flags for standard annotation reports (bold and bright pink).
        StdAnnotation   = (Bold | BrightPink),
    };
};


/* ----- Structures ----- */

/**
\brief Log color code structure.
\see Printf(const ColorCodes&, const char*, ...)
\see Errorf(const ColorCodes&, const char*, ...)
*/
struct ColorCodes
{
    ColorCodes() = default;
    ColorCodes(const ColorCodes&) = default;
    ColorCodes& operator = (const ColorCodes&) = default;

    //! Initializes only the text flags.
    inline ColorCodes(long textFlags) :
        textFlags { textFlags }
    {
    }

    //! Initializes both text and background flags.
    inline ColorCodes(long textFlags, long backgroundFlags) :
        textFlags       { textFlags       },
        backgroundFlags { backgroundFlags }
    {
    }

    /**
    \brief Bitwise OR combination of font flags for console text.
    \see Log::ColorFlags
    */
    long textFlags          = 0;

    /**
    \brief Bitwise OR combination of font flags for console background.
    \see Log::ColorFlags
    */
    long backgroundFlags    = 0;
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
using ReportCallbackExt = std::function<void(ReportType type, const char* text, void* userData, const ColorCodes& colors)>;


/* ----- Functions ----- */

/**
\brief Prints a formatted message to the log.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Printf
*/
LLGL_EXPORT void Printf(const char* format, ...);

/**
\brief Prints a formatted message to the log with color codes.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\param[in] colors Specifies color codes to highlight the printed text.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Printf
*/
LLGL_EXPORT void Printf(const ColorCodes& colors, const char* format, ...);

/**
\brief Prints a formatted error message to the log.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Errorf
*/
LLGL_EXPORT void Errorf(const char* format, ...);

/**
\brief Prints a formatted error message to the log with color codes.
\param[in] format Specifies the formatted text. Same as \c ::printf.
\param[in] colors Specifies color codes to highlight the printed text.
\remarks If this is called recursively, i.e. inside another log callback function, this function has no effect.
\see Report::Errorf
*/
LLGL_EXPORT void Errorf(const ColorCodes& colors, const char* format, ...);

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

//! \see RegisterCallback(const ReportCallback&, void*)
LLGL_EXPORT LogHandle RegisterCallback(const ReportCallbackExt& callback, void* userData = nullptr);

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
\param[in] stdOutFlags Bitwise OR combination of standard output flags (StdOutFlags). Use this to enable colored output for instance.
\returns Opaque handle to this log callback or null if this is called recursively, i.e. inside another log callback function.
It can be used to unregister the callback.
\remarks If there already is a registered handle for the standard output,
this function only returns the previously registered handle that is associated with the standard output.
\see SetReportCallback
\see StdOutFlags
*/
LLGL_EXPORT LogHandle RegisterCallbackStd(long stdOutFlags = 0);

/**
\brief Unregisters the specified handle from the log output.
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
