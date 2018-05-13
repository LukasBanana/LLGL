/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
    //!< Specifies whether vertical-synchronisation (Vsync) is enabled or disabled. By default disabled.
    bool            enabled     = false;

    //!< Refresh rate (in Hz). By default 60.
    std::uint32_t   refreshRate = 60;

    /**
    \brief Synchronisation interval. Can be 1, 2, 3, or 4.
    \remarks If Vsync is disabled, this value is implicit zero.
    */
    std::uint32_t   interval    = 1;
};

//! Video mode descriptor structure.
struct VideoModeDescriptor
{
    //! Screen resolution (in pixels).
    Size            resolution;

    /**
    \brief Number of bits for each pixel in the color buffer. Should be 24 or 32. By default 32.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the color buffer.
    */
    int             colorBits       = 32;

    /**
    \breif Number of bits for each pixel in the depth buffer. Should be 24, 32, or zero to disable depth buffer. By default 24.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the depth buffer.
    */
    int             depthBits       = 24;

    /**
    \breif Number of bits for each pixel in the stencil buffer. Should be 8, or zero to disable stencil buffer. By default 8.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the stencil buffer.
    */
    int             stencilBits     = 8;

    /**
    \breif Specifies whether to enable fullscreen mode or windowed mode. By default windowed mode.
    */
    bool            fullscreen      = false;

    /**
    \brief Number of swap-chain buffers. By default 2 (for double-buffering).
    \remarks This is only a hint to the renderer and there is no guarantee how many buffers are finally used for the swap chain.
    Especially OpenGL does not support custom swap chain sizes.
    */
    std::uint32_t   swapChainSize   = 2;
};

//TODO: move this into RenderSystemDescriptor, and make GL context creation part of GLRenderSystem instead of each GLRenderContext instance
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
    MultiSamplingDescriptor multiSampling;  //!< Multi-sampling descriptor.
    VideoModeDescriptor     videoMode;      //!< Video mode descriptor.
    ProfileOpenGLDescriptor profileOpenGL;  //!< OpenGL profile descriptor (to switch between compatability or core profile).
    DebugCallback           debugCallback;  //!< Debuging callback function object.
};


/* ----- Operators ----- */

LLGL_EXPORT bool operator == (const VsyncDescriptor& lhs, const VsyncDescriptor& rhs);
LLGL_EXPORT bool operator != (const VsyncDescriptor& lhs, const VsyncDescriptor& rhs);

LLGL_EXPORT bool operator == (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);
LLGL_EXPORT bool operator != (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);


} // /namespace LLGL


#endif



// ================================================================================
