/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_FLAGS_H
#define LLGL_RENDER_CONTEXT_FLAGS_H


#include "Export.h"
#include "Types.h"
#include "GraphicsPipelineFlags.h"
#include <functional>
#include <cstdint>


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

/**
\brief OpenGL context profile enumeration.
\remarks Can be used to specify a specific OpenGL profile other than the default (i.e. compatibility profile).
*/
enum class OpenGLContextProfile
{
    CompatibilityProfile,   //!< OpenGL compatibility profile. This is the default.
    CoreProfile,            //!< OpenGL core profile.
    ESProfile,              //!< OpenGL ES profile. \todo This is incomplete, do not use!
};


/* ----- Structures ----- */

//! Vertical-synchronization (Vsync) descriptor structure.
struct VsyncDescriptor
{
    bool            enabled     = false;    //!< Specifies whether vertical-synchronisation (Vsync) is enabled or disabled. By default disabled.
    std::uint32_t   refreshRate = 60;       //!< Refresh rate (in Hz). By default 60.
    std::uint32_t   interval    = 1;        //!< Synchronisation interval. Can be 1, 2, 3, or 4. If Vsync is disabled, this value is implicit zero.
};

//! Video mode descriptor structure.
struct VideoModeDescriptor
{
    Size            resolution;                 //!< Screen resolution.
    int             colorDepth      = 32;       //!< Color bit depth. Should be 24 or 32. By default 32.
    bool            fullscreen      = false;    //!< Specifies whether to enable fullscreen mode or windowed mode. By default windowed mode.
    std::uint32_t   swapChainSize   = 2;        //!< Number of swap-chain buffers. By default 2 (for double-buffering).
};

/**
\brief OpenGL profile descriptor structure.
\note On MacOS the only supported OpenGL profiles are compatibility profile (for lagecy OpenGL before 3.0), 3.2 core profile, or 4.1 core profile.
*/
struct ProfileOpenGLDescriptor
{
    //! Specifies the requested OpenGL context profile. By default OpenGLContextProfile::CompatibilityProfile.
    OpenGLContextProfile    contextProfile  = OpenGLContextProfile::CompatibilityProfile;

    /**
    \brief Specifies the requested OpenGL context major version. By default -1 to indicate to use the highest version possible.
    \remarks This member is ignored if 'contextProfile' is 'OpenGLContextProfile::CompatibilityProfile'.
    */
    int                     majorVersion    = -1;

    /**
    \brief Specifies the requested OpenGL context minor version. By default -1 to indicate to use the highest version possible.
    \remarks This member is ignored if 'contextProfile' is 'OpenGLContextProfile::CompatibilityProfile'.
    */
    int                     minorVersion    = -1;
};

//! Render context descriptor structure.
struct RenderContextDescriptor
{
    VsyncDescriptor         vsync;          //!< Vertical-synchronization (Vsync) descriptor.
    MultiSamplingDescriptor multiSampling;  //!< Sampling descriptor.
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
