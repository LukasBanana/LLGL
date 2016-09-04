/*
 * RenderContextDescriptor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_DESCRIPTOR_H__
#define __LLGL_RENDER_CONTEXT_DESCRIPTOR_H__


#include "Export.h"
#include "Types.h"
#include <functional>


namespace LLGL
{


/* ----- Types ----- */

/**
\brief Debug callback function interface.
\param[in] type Descriptive type of the message.
\param[in] message Specifies the debug output message.
\remarks This output is renderer dependent.
*/
using DebugCallback = std::function<void(const std::string& type, const std::string& message)>;


/* ----- Enumerations ----- */

enum class OpenGLVersion
{
    OpenGL_Latest   =   0, //!< Latest available OpenGL version (on the host platform).
    OpenGL_1_0      = 100, //!< OpenGL 1.0, released in Jan, 1992.
    OpenGL_1_1      = 110, //!< OpenGL 1.1, released in Mar, 1997.
    OpenGL_1_2      = 120, //!< OpenGL 1.2, released in Mar, 1998.
    OpenGL_1_3      = 130, //!< OpenGL 1.3, released in Aug, 2001.
    OpenGL_1_4      = 140, //!< OpenGL 1.4, released in Jul, 2002.
    OpenGL_1_5      = 150, //!< OpenGL 1.5, released in Jul, 2003.
    OpenGL_2_0      = 200, //!< OpenGL 2.0, released in Sep, 2004.
    OpenGL_2_1      = 210, //!< OpenGL 2.1, released in Jul, 2006.
    OpenGL_3_0      = 300, //!< OpenGL 3.0, released in Aug, 2008 (known as "Longs Peak").
    OpenGL_3_1      = 310, //!< OpenGL 3.1, released in Mar, 2009 (known as "Longs Peak Reloaded").
    OpenGL_3_2      = 320, //!< OpenGL 3.2, released in Aug, 2009.
    OpenGL_3_3      = 330, //!< OpenGL 3.3, released in Mar, 2010.
    OpenGL_4_0      = 400, //!< OpenGL 4.0, released in Mar, 2010 (alongside with OpenGL 3.3).
    OpenGL_4_1      = 410, //!< OpenGL 4.1, released in Jul, 2010.
    OpenGL_4_2      = 420, //!< OpenGL 4.2, released in Aug, 2011.
    OpenGL_4_3      = 430, //!< OpenGL 4.3, released in Aug, 2012.
    OpenGL_4_4      = 440, //!< OpenGL 4.4, released in Jul, 2013.
    OpenGL_4_5      = 450, //!< OpenGL 4.5, released in Aug, 2014.
};

//! Swap chain mode enumeration.
enum class SwapChainMode
{
    SingleBuffering = 1,    //!< Single buffering. This is almost no longer used.
    DoubleBuffering,        //!< Double buffering. This is the default for most renderers.
    TripleBuffering,        //!< Triple buffering. Triple buffering can only be used for Direct3D renderers.
};


/* ----- Structures ----- */

struct VsyncDescriptor
{
    bool            enabled     = false;    //!< Specifies whether vertical-synchronisation (Vsync) is enabled or disabled. By default disabled.
    unsigned int    refreshRate = 60;       //!< Refresh rate (in Hz). By default 60.
    unsigned int    interval    = 1;        //!< Synchronisation interval. Can be 1, 2, 3, or 4. If Vsync is disabled, this value is implicit zero.
};

struct AntiAliasingDescriptor
{
    bool            enabled     = false;    //!< Specifies whether multi-sampling is enabled or disabled. By default disabled.
    unsigned int    samples     = 1;        //!< Number of samples used for multi-sampling. By default 1.
};

struct VideoModeDescriptor
{
    Size            resolution;                                         //!< Screen resolution.
    int             colorDepth      = 32;                               //!< Color bit depth. Should be 24 or 32. By default 32.
    bool            fullscreen      = false;                            //!< Specifies whether to enable fullscreen mode or windowed mode. By default windowed mode.
    SwapChainMode   swapChainMode   = SwapChainMode::DoubleBuffering;   //!< Swap chain buffering mode.
};

struct ProfileOpenGLDescriptor
{
    //! Specifies whether an extended renderer profile is to be used. By default false.
    bool            extProfile  = false;

    /**
    \brief Specifies whether to use 'OpenGL Core Profile', instead of 'OpenGL Compatibility Profile'. By default disbaled.
    \remarks This requires 'extProfile' to be enabled.
    */
    bool            coreProfile = false;

    //! Specifies whether the hardware renderer will produce debug dump. By default disabled.
    bool            debugDump   = false;

    /**
    \brief OpenGL version to create the render context with.
    \remarks This required 'coreProfile' to be enabled.
    */
    OpenGLVersion   version     = OpenGLVersion::OpenGL_Latest;
};

struct RenderContextDescriptor
{
    VsyncDescriptor         vsync;          //!< Vertical-synchronization (Vsync) descriptor.
    AntiAliasingDescriptor  antiAliasing;   //!< Multi-sample anti-aliasing descriptor.
    VideoModeDescriptor     videoMode;      //!< Video mode descriptor.
    ProfileOpenGLDescriptor profileOpenGL;  //!< OpenGL profile descriptor (to switch between compatability or core profile).
    DebugCallback           debugCallback;  //!< Debuging callback descriptor.
};


/* ----- Operators ----- */

LLGL_EXPORT bool operator == (const VsyncDescriptor& lhs, const VsyncDescriptor& rhs);
LLGL_EXPORT bool operator != (const VsyncDescriptor& lhs, const VsyncDescriptor& rhs);

LLGL_EXPORT bool operator == (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);
LLGL_EXPORT bool operator != (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);


} // /namespace LLGL


#endif



// ================================================================================
