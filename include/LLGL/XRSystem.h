/*
 * XRSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_SYSTEM_H
#define LLGL_XR_SYSTEM_H


#include <LLGL/Interface.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Report.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/XRSession.h>
#include <LLGL/XRSwapChain.h>
#include <LLGL/XRSystemFlags.h>

#include <cstdint>
#include <memory>


namespace LLGL
{


class XRSystem;

/**
\brief Unique pointer type for the XRSystem interface.
\see XRSystem::Load
\see XRSystem::Unload
*/
using XRSystemPtr = std::unique_ptr<XRSystem>;

/**
\brief XR system interface.
\remarks Top-level entry point for OpenXR support. An XRSystem brings up the XR runtime, queries the
selected device's view configuration, and prepares an LLGL::RenderSystem whose underlying graphics device
satisfies the runtime's binding requirements.

Typical usage:
\code
// 1. Bring up XR runtime
LLGL::XRSystemDescriptor xrDesc;
xrDesc.applicationName = "MyApp";
LLGL::XRSystemPtr xr = LLGL::XRSystem::Load(xrDesc);

// 2. Create an XR-compatible renderer
LLGL::XRRenderSystemDescriptor rDesc;
rDesc.rendererModule = "Vulkan";
LLGL::RenderSystemPtr renderer = xr->CreateRenderSystem(rDesc);

// 3. Create an XR session bound to that renderer, then per-view swap-chains
LLGL::XRSession* session = xr->CreateSession(LLGL::XRSessionDescriptor{}, *renderer);
const auto views = xr->GetViewConfigurations();
//...
\endcode
\see XRSession
\see XRSwapChain
*/
class LLGL_EXPORT XRSystem : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::XRSystem );

    public:

        //! Releases internal data.
        ~XRSystem();

        /**
        \brief Loads and brings up an XR runtime.
        \param[in] xrSystemDesc Specifies the XR system descriptor.
        \param[out] report Optional pointer to a report on failure of loading the XR runtime.
        \return Unique pointer to the new XR system, or null on failure (in which case \c report is populated if provided).
        \remarks This function discovers the active OpenXR runtime via the OpenXR loader and queries the
        selected form factor. Returns null if no compatible runtime is installed or the requested form factor is unavailable.
        */
        static XRSystemPtr Load(const XRSystemDescriptor& xrSystemDesc, Report* report = nullptr);

        /**
        \brief Unloads the specified XR system.
        \remarks After this call, all sessions and swap-chains created from this system must no longer be used.
        */
        static void Unload(XRSystemPtr&& xrSystem);

    public:

        //! Returns the runtime name reported by the active OpenXR runtime (e.g. "SteamVR" or "Monado").
        virtual const char* GetRuntimeName() const = 0;

        /**
        \brief Returns the view configuration entries for the form factor selected at load time.
        \remarks The returned array's size is the number of views the application must render each frame
        (e.g. 2 for stereo). Each entry carries recommended and maximum image dimensions plus sample counts.
        */
        virtual ArrayView<XRViewConfigurationView> GetViewConfigurations() const = 0;

        //! Returns a pointer to the report or null if there is none.
        virtual const Report* GetReport() const = 0;

    public:

        /**
        \brief Creates an LLGL::RenderSystem whose underlying graphics device satisfies the XR runtime's binding requirements.
        \param[in] renderSystemDesc Specifies the XR render system descriptor.
        \param[out] report Optional pointer to a report on failure.
        \return Unique pointer to a render system, or null on failure.
        \remarks Currently only \c renderSystemDesc.rendererModule = "Vulkan" is supported. The returned render
        system must be used as the renderer parameter to CreateSession.
        \see XRRenderSystemDescriptor
        */
        virtual RenderSystemPtr CreateRenderSystem(const XRRenderSystemDescriptor& renderSystemDesc, Report* report = nullptr) = 0;

        /**
        \brief Creates a new XR session bound to the given render system.
        \param[in] sessionDesc Specifies the XR session descriptor.
        \param[in] renderSystem The renderer to bind the session to. Must have been obtained from CreateRenderSystem
        on this same XRSystem instance.
        \return Pointer to the new XRSession, or null on failure.
        \remarks Ownership of the session remains with the XRSystem; use Release to free it.
        */
        virtual XRSession* CreateSession(const XRSessionDescriptor& sessionDesc, RenderSystem& renderSystem) = 0;

        //! Releases the specified session. After this call, the session must no longer be used.
        virtual void Release(XRSession& session) = 0;

    public:

        /**
        \brief Polls the OpenXR runtime for events.
        \remarks Must be called regularly from the main loop; this drives session-state transitions reported
        through XRSession::GetState. Returns true if any events were processed.
        */
        virtual bool PollEvents() = 0;

    public:

        /**
        \brief Returns the native handle of this XR system.
        \param[out] nativeHandle Pointer to the backend-specific native handle structure.
        Obtain it from <code>#include <LLGL/Backend/OpenXR/NativeHandle.h></code>.
        \param[in] nativeHandleSize Size (in bytes) of the native handle structure.
        \return True if the native handle was successfully retrieved.
        \see OpenXR::SystemNativeHandle
        */
        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) = 0;

    protected:

        //! Allocates the internal data.
        XRSystem();

};


} // /namespace LLGL


#endif



// ================================================================================
