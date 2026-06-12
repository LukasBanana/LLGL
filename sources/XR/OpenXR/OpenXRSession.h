/*
 * OpenXRSession.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_SESSION_H
#define LLGL_OPENXR_SESSION_H


#include <LLGL/XR/XRSession.h>
#include <LLGL/Report.h>
#include <LLGL/Container/SmallVector.h>

#include "OpenXRPlatform.h"
#include "OpenXRSwapChain.h"
#include "../../Renderer/ContainerTypes.h"

#include <cstdint>
#include <memory>
#include <vector>


namespace LLGL
{


class RenderSystem;


namespace OpenXR
{


class OpenXRSystem;
class GraphicsBinding;

class OpenXRSession final : public XRSession
{

    public:

        OpenXRSession(
            OpenXRSystem&               owner,
            GraphicsBinding&            binding,
            RenderSystem&               renderSystem,
            XrSession                   session,
            XrSpace                     referenceSpace,
            XrViewConfigurationType     viewConfigurationType,
            XrEnvironmentBlendMode      environmentBlendMode,
            std::uint32_t               viewCount,
            SmallVector<std::int64_t>&& runtimeFormats,
            bool                        depthSubmissionEnabled
        );

        ~OpenXRSession();

    public:

        XRSessionState              GetState() const override;
        XREnvironmentBlendMode      GetEnvironmentBlendMode() const override;
        ArrayView<Format>           GetSupportedColorFormats() const override;
        ArrayView<Format>           GetSupportedDepthFormats() const override;

        XRSwapChain*                CreateSwapChain(const XRSwapChainDescriptor& swapChainDesc) override;
        void                        Release(XRSwapChain& swapChain) override;

        bool                        IsRunning() const override;
        bool                        WaitFrame(XRFrameState &frameState) override;
        bool                        BeginFrame() override;
        bool                        EndFrame(float nearZ, float farZ, ArrayView<XRSwapChain *> swapChains) override;

        bool                        GetViewState(DynamicVector<XRViewPose> &viewsOut) override;
        bool                        GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) override;

        const Report*               GetReport() const override;

    public:

        //! Called by OpenXRSystem when polling events. Updates the cached session state and may issue xrBeginSession/xrEndSession.
        void HandleSessionStateChanged(XrSessionState newState);

        //! Returns the underlying OpenXR session handle.
        XrSession GetSession() const { return session_; }

        //! Returns the reference space used for view-pose location.
        XrSpace GetReferenceSpace() const { return referenceSpace_; }

    private:

        //! Creates the native XR swap-chain for the requested format/resolution and enumerates its images as LLGL textures.
        //! Returns false (and writes report_) if the format is unsupported or creation fails.
        bool CreateNativeSwapChain(
            const XRSwapChainDescriptor&    swapChainDesc,
            XrSwapchain&                    outSwapchain,
            std::int64_t&                   outNativeFormat,
            XRSwapChainDescriptor&          outEffectiveDesc,
            std::vector<XRSwapchainImage>&  outImages
        );

    private:

        OpenXRSystem&                                       owner_;
        GraphicsBinding&                                    binding_;
        RenderSystem&                                       renderSystem_;
        XrSession                                           session_                = XR_NULL_HANDLE;
        XrSpace                                             referenceSpace_         = XR_NULL_HANDLE;
        XrViewConfigurationType                             viewConfigurationType_;
        XrEnvironmentBlendMode                              environmentBlendMode_;
        std::uint32_t                                       viewCount_              = 0;
        SmallVector<std::int64_t>                           runtimeFormats_;
        SmallVector<Format>                                 supportedColorFormats_;
        SmallVector<Format>                                 supportedDepthFormats_;
        bool                                                depthSubmissionEnabled_ = false;
        HWObjectContainer<OpenXRSwapChain>                  ownedSwapChains_;
        Report                                              report_;

        XrSessionState                                      state_                  = XR_SESSION_STATE_UNKNOWN;
        bool                                                running_                = false;
        bool                                                frameStarted_           = false;
        XrFrameState                                        currentFrameState_;
        SmallVector<XrView>                                 currentViews_;
};


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
