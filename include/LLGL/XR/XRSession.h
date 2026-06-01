/*
 * XRSession.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_SESSION_H
#define LLGL_XR_SESSION_H


#include <LLGL/Interface.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/DynamicVector.h>
#include <LLGL/Format.h>
#include <LLGL/Report.h>
#include <LLGL/XR/XRSwapChain.h>
#include <LLGL/XR/XRSystemFlags.h>

#include <cstdint>


namespace LLGL
{


/**
\brief Per-frame state returned by XRSession::BeginFrame.
\see XRSession::BeginFrame
\see XRSession::EndFrame
*/
struct XRFrameState
{
    //! Runtime-predicted display time for this frame, in nanoseconds.
    std::int64_t                predictedDisplayTimeNs  = 0;

    //! True if the application should render this frame. If false, BeginFrame/EndFrame must still be paired but no rendering work is needed.
    bool                        shouldRender            = false;
};

/**
\brief XR session lifecycle states reported by XRSession::GetState.
\see XRSession::GetState
*/
enum class XRSessionState
{
    //! Session has been created but has not yet been started by the runtime.
    Idle,

    //! Session is ready; BeginFrame/EndFrame may be called.
    Ready,

    //! Session is synchronizing with the runtime; calls to BeginFrame may not yet render.
    Synchronized,

    //! Session is visible to the user and rendering should occur.
    Visible,

    //! Session is focused; the user is actively interacting with it.
    Focused,

    //! Session has been requested to stop. Application should finish in-flight frames and call EndSession.
    Stopping,

    //! Session has lost its runtime context (e.g. headset disconnected). The session is no longer usable.
    LossPending,

    //! Session is being torn down.
    Exiting,
};

/**
\brief XR session interface.
\remarks Drives the per-frame loop: BeginFrame yields predicted view poses, the application renders into one
swap-chain image per view, and EndFrame submits the projection layer back to the runtime.
\see XRSystem::CreateSession
*/
class LLGL_EXPORT XRSession : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::XRSession );

    public:

        //! Releases internal data. The owning XRSystem releases native XR resources.
        ~XRSession();

    public:

        //! Returns the current runtime-reported session state.
        virtual XRSessionState GetState() const = 0;

        //! Returns the environment blend mode the runtime selected for this session.
        virtual XREnvironmentBlendMode GetEnvironmentBlendMode() const = 0;

        /**
        \brief Returns the list of color formats supported by this session for swap-chains, in runtime-preferred order.
        \remarks The application should pick the first format that satisfies its rendering needs.
        \see CreateSwapChain
        */
        virtual ArrayView<Format> GetSupportedColorFormats() const = 0;

        /**
        \brief Returns the list of depth-stencil formats supported by this session for swap-chains, in runtime-preferred order.
        \remarks Empty if the runtime doesn't support depth swap-chain submission (i.e. \c XR_KHR_composition_layer_depth is unavailable).
        Use the returned format in XRSwapChainDescriptor::format when creating a depth companion swap-chain.
        \see CreateSwapChain
        \see XRSwapChain::SetDepthCompanion
        */
        virtual ArrayView<Format> GetSupportedDepthFormats() const = 0;

        /**
        \brief Creates a new XR swap-chain.
        \param[in] swapChainDesc Specifies the swap-chain descriptor.
        \return Pointer to the new XRSwapChain or null on failure (see XRSystem::GetReport).
        \remarks The session retains ownership of the swap-chain; releasing it through ReleaseSwapChain or destroying the session frees the underlying XR resources.
        */
        virtual XRSwapChain* CreateSwapChain(const XRSwapChainDescriptor& swapChainDesc) = 0;

        //! Releases the specified swap-chain. After this call, the specified object must no longer be used.
        virtual void Release(XRSwapChain& swapChain) = 0;

    public:

        /**
        \brief Returns true if the session has been signalled ready by the runtime and BeginFrame may be invoked.
        \remarks This becomes true after the runtime transitions to XRSessionState::Ready.
        Most applications should not call BeginFrame/EndFrame while this returns false.
        */
        virtual bool IsRunning() const = 0;

        /**
         \brief Waits for the previous frame to finish and retrieves the next frame state. Must be called before BeginFrame
         at the beginning of the app's update.
         \param[out] frameState Receives the predicted display time and a flag indicating whether the application should render.
         \return True on success
         */
        virtual bool WaitFrame(XRFrameState &frameState) = 0;

        /**
        \brief Begins a new XR frame.  Must be called after WaitFrame()
        \return True on success. On failure, the frame must be skipped (do not call EndFrame).
        */
        virtual bool BeginFrame() = 0;

        /**
        \brief Submits a frame to the runtime.
        \param[in] nearZ Near clipping plane (in meters) the application used to render this frame.
        \param[in] farZ Far clipping plane (in metres) the application used to render this frame.
        \param[in] swapChains One swap-chain per view, in the same order as XRSystem::GetViewConfigurations().
        \return True on success.
        \remarks This submits a projection composition layer using the swap-chains' currently released images.
        If WaitFrame returned false in \c frameState.shouldRender, an empty layer set is submitted instead (the runtime still requires a paired EndFrame call).
        */
        virtual bool EndFrame(float nearZ, float farZ, ArrayView<XRSwapChain*> swapChains) = 0;

        /**
         \brief Sample and return the latest predicted poses for the session's views.
         \param[out] viewsOut Receives the view poses and projection parameters. The size of this array matches the view count
         returned by XRSystem::GetViewConfigurations().
         \return True on success.
         \remarks Each time this is called, the runtime updates the view poses based on the latest predicted display time. The
         application should call this at least once per frame after WaitFrame() to get the latest view poses for rendering and
         reprojection. If EndFrame is called with a non-empty swap-chain array, the runtime also uses these poses for reprojection
         of the submitted layer.
         */
        virtual bool GetViewState(DynamicVector<XRViewPose> &viewsOut) = 0;

    public:

        /**
        \brief Returns the native handle of this XR Session.
        \param[out] nativeHandle Pointer to the backend-specific native handle structure.
        Obtain it from <code>#include <LLGL/Backend/OpenXR/NativeHandle.h></code>.
        \param[in] nativeHandleSize Size (in bytes) of the native handle structure.
        \return True if the native handle was successfully retrieved.
        \see OpenXR::SessionNativeHandle
        */
        virtual bool GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) = 0;

    public:

        /**
        \brief Returns a pointer to the report or null if there is none.
        \remarks Indicates errors from a previous session operation.
        \see Report
        */
        virtual const Report* GetReport() const = 0;

    protected:

        //! Allocates the internal data.
        XRSession();

};


} // /namespace LLGL


#endif



// ================================================================================
