/*
 * XRSystemFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_SYSTEM_FLAGS_H
#define LLGL_XR_SYSTEM_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Format.h>
#include <LLGL/Types.h>
#include <LLGL/Container/StringLiteral.h>
#include <LLGL/Container/UTF8String.h>

#include <LLGL/Platform/Platform.h>
#if defined LLGL_OS_ANDROID
#   include <android_native_app_glue.h>
#endif

#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief XR form factor enumeration.
\remarks Selects the kind of XR device the application targets.
The runtime may report a chosen form factor as unavailable, in which case XRSystem::Load returns null with a report.
\see XRSystemDescriptor::formFactor
*/
enum class XRFormFactor
{
    //! Head-mounted display, e.g. a VR headset.
    HeadMountedDisplay,

    //! Handheld display, e.g. a phone or tablet held by the user.
    HandheldDisplay,
};

/**
\brief XR view configuration enumeration.
\remarks Selects how many views are rendered each frame and how they are laid out.
\see XRSystemDescriptor::viewConfiguration
\see XRSystem::GetViewConfigurations
*/
enum class XRViewConfiguration
{
    //! A single view, typically used for non-stereo handheld or pass-through scenarios.
    Mono,

    //! Two views, one per eye, in head-mounted display configurations.
    Stereo,
};

/**
\brief XR environment blend mode enumeration.
\remarks Describes how the runtime composes the rendered image against the physical world.
\see XRSessionDescriptor::environmentBlendMode
*/
enum class XREnvironmentBlendMode
{
    //! Output is fully opaque; the physical environment is occluded entirely.
    Opaque,

    //! Output is blended additively over a view of the physical environment.
    Additive,

    //! Output is alpha-blended over a view of the physical environment (passthrough).
    AlphaBlend,
};


/* ----- Flags ----- */

/**
\brief XR system flags enumeration.
\see XRSystemDescriptor::flags
*/
struct XRSystemFlags
{
    enum
    {
        /**
        \brief Requests that the OpenXR runtime install a debug messenger so validation output is forwarded to the LLGL log.
        \remarks This is only a hint; not all runtimes expose the debug-utils extension.
        */
        DebugDevice = (1 << 0),
    };
};


/* ----- Structures ----- */

/**
\brief XR system descriptor structure.
\see XRSystem::Load
*/
struct XRSystemDescriptor
{
    //! Application name reported to the OpenXR runtime. Defaults to "LLGL Application".
    UTF8String              applicationName     = "LLGL Application";

    //! Application version reported to the OpenXR runtime. Defaults to 1.
    std::uint32_t           applicationVersion  = 1;

    //! Engine name reported to the OpenXR runtime. Defaults to "LLGL".
    UTF8String              engineName          = "LLGL";

    //! Engine version reported to the OpenXR runtime. Defaults to 1.
    std::uint32_t           engineVersion       = 1;

    //! Selected form factor. Defaults to XRFormFactor::HeadMountedDisplay.
    XRFormFactor            formFactor          = XRFormFactor::HeadMountedDisplay;

    //! Selected view configuration. Defaults to XRViewConfiguration::Stereo.
    XRViewConfiguration     viewConfiguration   = XRViewConfiguration::Stereo;

    //! Combination of XRSystemFlags entries. By default 0.
    long                    flags               = 0;

    #ifdef LLGL_OS_ANDROID

    /**
    \brief Android-specific application descriptor. \b Required on Android.
    \remarks This must point to the \c android_app structure delivered to the application's
    \c android_main(android_app*) entry point. The OpenXR loader uses the embedded JavaVM
    and Activity instance to discover and bind to the system XR runtime via
    \c xrInitializeLoaderKHR and \c XrInstanceCreateInfoAndroidKHR.
    \note Only required on: Android.
    */
    android_app*            androidApp          = nullptr;

    #endif // /LLGL_OS_ANDROID
};

/**
\brief XR render system descriptor structure.
\remarks Used to obtain an LLGL::RenderSystem whose underlying device satisfies the XR runtime's requirements.
\see XRSystem::CreateRenderSystem
*/
struct XRRenderSystemDescriptor
{
    /**
    \brief Specifies which LLGL renderer module the XR runtime should bind to.
    \remarks Currently only \c "Vulkan" is supported.
    */
    StringLiteral           rendererModule;

    /**
    \brief Combination of LLGL::RenderSystemFlags entries forwarded to the underlying renderer.
    \see RenderSystemFlags
    */
    long                    flags               = 0;

    /**
    \brief Optional pointer to a renderer-specific configuration structure.
    \see RenderSystemDescriptor::rendererConfig
    */
    const void*             rendererConfig      = nullptr;

    //! Size (in bytes) of the structure \c rendererConfig points to. Ignored if \c rendererConfig is null.
    std::size_t             rendererConfigSize  = 0;

    #ifdef LLGL_OS_ANDROID

    /**
    \brief Android-specific application descriptor forwarded to the underlying renderer.
    \remarks On Android, XRSystem auto-fills this from the value passed to XRSystemDescriptor::androidApp,
    so applications normally do not need to set it explicitly. Forwarded into RenderSystemDescriptor::androidApp
    when the underlying renderer module is loaded.
    */
    android_app*            androidApp          = nullptr;

    #endif // /LLGL_OS_ANDROID
};

/**
\brief XR session descriptor structure.
\see XRSystem::CreateSession
*/
struct XRSessionDescriptor
{
    /**
    \brief Specifies the requested environment blend mode for the session's projection layer.
    \remarks If the runtime does not support the requested blend mode it picks the first available one and the chosen mode is reported back via XRSession::GetEnvironmentBlendMode.
    */
    XREnvironmentBlendMode  environmentBlendMode    = XREnvironmentBlendMode::Opaque;
};

/**
\brief Per-view configuration entry returned by XRSystem::GetViewConfigurations.
\see XRSystem::GetViewConfigurations
*/
struct XRViewConfigurationView
{
    //! Recommended image width (in pixels) for this view's swap-chain.
    std::uint32_t   recommendedImageWidth   = 0;

    //! Recommended image height (in pixels) for this view's swap-chain.
    std::uint32_t   recommendedImageHeight  = 0;

    //! Maximum supported image width (in pixels) for this view's swap-chain.
    std::uint32_t   maxImageWidth           = 0;

    //! Maximum supported image height (in pixels) for this view's swap-chain.
    std::uint32_t   maxImageHeight          = 0;

    //! Recommended sample count for this view's swap-chain. Use this for MSAA targets.
    std::uint32_t   recommendedSampleCount  = 1;

    //! Maximum supported sample count for this view's swap-chain.
    std::uint32_t   maxSampleCount          = 1;
};

/**
\brief XR swap-chain descriptor structure.
\see XRSession::CreateSwapChain
*/
struct XRSwapChainDescriptor
{
    //! Color format for the swap-chain images. Must be supported by the runtime (see XRSession::GetSupportedColorFormats).
    Format          format          = Format::BGRA8UNorm;

    //! Pixel resolution of the swap-chain.
    Extent2D        resolution;

    //! Sample count for multi-sampled targets. Defaults to 1.
    std::uint32_t   sampleCount     = 1;

    //! Number of array layers. Defaults to 1. Use 2 for multiview-style stacked targets if supported.
    std::uint32_t   arrayLayers     = 1;
};

/**
\brief Pose and projection parameters for a single XR view sampled at frame begin.
\see XRFrameState::views
*/
struct XRViewPose
{
    //! View orientation as a unit quaternion (x, y, z, w).
    float           orientation[4]      = { 0.0f, 0.0f, 0.0f, 1.0f };

    //! View position in meters relative to the session's reference space (x, y, z).
    float           position[3]         = { 0.0f, 0.0f, 0.0f };

    //! Left edge of the projection frustum in radians (negative for left of forward).
    float           fovAngleLeft        = 0.0f;

    //! Right edge of the projection frustum in radians.
    float           fovAngleRight       = 0.0f;

    //! Top edge of the projection frustum in radians.
    float           fovAngleUp          = 0.0f;

    //! Bottom edge of the projection frustum in radians (negative below forward).
    float           fovAngleDown        = 0.0f;
};


} // /namespace LLGL


#endif



// ================================================================================
