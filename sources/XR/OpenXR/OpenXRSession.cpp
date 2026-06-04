/*
 * OpenXRSession.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "OpenXRSession.h"
#include "OpenXRSwapChain.h"
#include "OpenXRGraphicsBinding.h"
#include "OpenXRError.h"

#include <LLGL/RenderSystem.h>
#include <LLGL/Texture.h>
#include <LLGL/Backend/OpenXR/NativeHandle.h>

#include <cstring>
#include <memory>


namespace LLGL
{

namespace OpenXR
{


static XRSessionState TranslateSessionState(XrSessionState state)
{
    switch (state)
    {
        case XR_SESSION_STATE_IDLE:         return XRSessionState::Idle;
        case XR_SESSION_STATE_READY:        return XRSessionState::Ready;
        case XR_SESSION_STATE_SYNCHRONIZED: return XRSessionState::Synchronized;
        case XR_SESSION_STATE_VISIBLE:      return XRSessionState::Visible;
        case XR_SESSION_STATE_FOCUSED:      return XRSessionState::Focused;
        case XR_SESSION_STATE_STOPPING:     return XRSessionState::Stopping;
        case XR_SESSION_STATE_LOSS_PENDING: return XRSessionState::LossPending;
        case XR_SESSION_STATE_EXITING:      return XRSessionState::Exiting;
        default:                            return XRSessionState::Idle;
    }
}

static XREnvironmentBlendMode TranslateBlendMode(XrEnvironmentBlendMode mode)
{
    switch (mode)
    {
        case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      return XREnvironmentBlendMode::Opaque;
        case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    return XREnvironmentBlendMode::Additive;
        case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: return XREnvironmentBlendMode::AlphaBlend;
        default:                                    return XREnvironmentBlendMode::Opaque;
    }
}


OpenXRSession::OpenXRSession(
    OpenXRSystem&                   owner,
    GraphicsBinding&                binding,
    RenderSystem&                   renderSystem,
    XrSession                       session,
    XrSpace                         referenceSpace,
    XrViewConfigurationType         viewConfigurationType,
    XrEnvironmentBlendMode          environmentBlendMode,
    std::uint32_t                   viewCount,
    SmallVector<std::int64_t>&&     runtimeFormats,
    bool                            depthSubmissionEnabled)
:
    owner_                      { owner                 },
    binding_                    { binding               },
    renderSystem_               { renderSystem          },
    session_                    { session               },
    referenceSpace_             { referenceSpace        },
    viewConfigurationType_      { viewConfigurationType },
    environmentBlendMode_       { environmentBlendMode  },
    viewCount_                  { viewCount             },
    runtimeFormats_             { std::move(runtimeFormats) },
    depthSubmissionEnabled_     { depthSubmissionEnabled },
    state_                      { XR_SESSION_STATE_IDLE }
{
    currentViews_.resize(viewCount_, XrView{XR_TYPE_VIEW});

    // Bucket the runtime's mixed list of swap-chain formats into the LLGL-format views the
    // application can pick from: color formats via SelectColorFormat, depth formats via SelectDepthFormat.
    supportedColorFormats_.reserve(runtimeFormats_.size());
    supportedDepthFormats_.reserve(runtimeFormats_.size());
    for (auto nativeFormat : runtimeFormats_)
    {
        std::int64_t echoed = 0;
        const Format asColor = binding_.SelectColorFormat(
            ArrayView<std::int64_t>{ &nativeFormat, 1 }, Format::Undefined, echoed);
        if (asColor != Format::Undefined)
        {
            supportedColorFormats_.push_back(asColor);
            continue;
        }
        const Format asDepth = binding_.SelectDepthFormat(
            ArrayView<std::int64_t>{ &nativeFormat, 1 }, Format::Undefined, echoed);
        if (asDepth != Format::Undefined)
            supportedDepthFormats_.push_back(asDepth);
    }
}

OpenXRSession::~OpenXRSession()
{
    ownedSwapChains_.clear();

    if (referenceSpace_ != XR_NULL_HANDLE)
        xrDestroySpace(referenceSpace_);

    if (session_ != XR_NULL_HANDLE)
    {
        if (running_)
            xrEndSession(session_);
        xrDestroySession(session_);
    }
}

XRSessionState OpenXRSession::GetState() const
{
    return TranslateSessionState(state_);
}

XREnvironmentBlendMode OpenXRSession::GetEnvironmentBlendMode() const
{
    return TranslateBlendMode(environmentBlendMode_);
}

ArrayView<Format> OpenXRSession::GetSupportedColorFormats() const
{
    return ArrayView<Format>{ supportedColorFormats_.data(), supportedColorFormats_.size() };
}

ArrayView<Format> OpenXRSession::GetSupportedDepthFormats() const
{
    if (!depthSubmissionEnabled_)
        return ArrayView<Format>{};
    return ArrayView<Format>{ supportedDepthFormats_.data(), supportedDepthFormats_.size() };
}

bool OpenXRSession::CreateNativeSwapChain(
    const XRSwapChainDescriptor&    swapChainDesc,
    XrSwapchain&                    outSwapchain,
    std::int64_t&                   outNativeFormat,
    XRSwapChainDescriptor&          outEffectiveDesc,
    std::vector<XRSwapchainImage>&  outImages)
{
    // The runtime returns one combined list of swap-chain formats; whether the requested format
    // is color or depth determines the XR usage flags and the LLGL bind flags we'll set later.
    std::int64_t nativeFormat = 0;
    Format selected = binding_.SelectColorFormat(
        ArrayView<std::int64_t>{ runtimeFormats_.data(), runtimeFormats_.size() },
        swapChainDesc.format,
        nativeFormat
    );
    GraphicsBinding::SwapchainKind kind = GraphicsBinding::SwapchainKind::Color;
    XrSwapchainUsageFlags usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;

    if (selected == Format::Undefined)
    {
        // Fall back to depth-format lookup.
        selected = binding_.SelectDepthFormat(
            ArrayView<std::int64_t>{ runtimeFormats_.data(), runtimeFormats_.size() },
            swapChainDesc.format,
            nativeFormat
        );
        if (selected != Format::Undefined)
        {
            kind = GraphicsBinding::SwapchainKind::DepthStencil;
            usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
    }

    if (selected == Format::Undefined)
    {
        report_.Errorf("XRSession::CreateSwapChain failed: requested format is not supported by runtime\n");
        return false;
    }

    XrSwapchainCreateInfo createInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    createInfo.usageFlags       = usageFlags;
    createInfo.format           = nativeFormat;
    createInfo.sampleCount      = swapChainDesc.sampleCount;
    createInfo.width            = swapChainDesc.resolution.width;
    createInfo.height           = swapChainDesc.resolution.height;
    createInfo.faceCount        = 1;
    createInfo.arraySize        = swapChainDesc.arrayLayers;
    createInfo.mipCount         = 1;

    XrSwapchain xrSwapchain = XR_NULL_HANDLE;
    const XrResult result = xrCreateSwapchain(session_, &createInfo, &xrSwapchain);
    if (Failed(result))
    {
        ReportXrError(&report_, XR_NULL_HANDLE, result, "xrCreateSwapchain");
        return false;
    }

    XRSwapChainDescriptor effectiveDesc = swapChainDesc;
    effectiveDesc.format = selected;

    std::vector<XRSwapchainImage> images;
    if (!binding_.EnumerateSwapchainImages(renderSystem_, xrSwapchain, effectiveDesc, nativeFormat, kind, images, &report_))
    {
        xrDestroySwapchain(xrSwapchain);
        return false;
    }

    outSwapchain     = xrSwapchain;
    outNativeFormat  = nativeFormat;
    outEffectiveDesc = effectiveDesc;
    outImages        = std::move(images);
    return true;
}

XRSwapChain* OpenXRSession::CreateSwapChain(const XRSwapChainDescriptor& swapChainDesc)
{
    // 1. Create the color swap-chain (the runtime owns its images).
    XrSwapchain                     colorSwapchain      = XR_NULL_HANDLE;
    std::int64_t                    colorNativeFormat   = 0;
    XRSwapChainDescriptor           colorEffectiveDesc;
    std::vector<XRSwapchainImage>   colorImages;
    if (!CreateNativeSwapChain(swapChainDesc, colorSwapchain, colorNativeFormat, colorEffectiveDesc, colorImages))
        return nullptr;

    OpenXRSwapChain* swapChain = ownedSwapChains_.emplace<OpenXRSwapChain>(
        binding_, renderSystem_, colorSwapchain, colorEffectiveDesc, colorNativeFormat, std::move(colorImages));

    // 2. Provision a managed depth buffer if one was requested.
    if (swapChainDesc.depthStencilFormat != Format::Undefined)
    {
        bool depthReady = false;

        // Prefer a depth swap-chain submitted to the runtime for reprojection, if the runtime supports it.
        if (depthSubmissionEnabled_)
        {
            XRSwapChainDescriptor depthDesc;
            depthDesc.format        = swapChainDesc.depthStencilFormat;
            depthDesc.resolution    = swapChainDesc.resolution;
            depthDesc.sampleCount   = swapChainDesc.sampleCount;
            depthDesc.arrayLayers   = swapChainDesc.arrayLayers;

            XrSwapchain                     depthSwapchain      = XR_NULL_HANDLE;
            std::int64_t                    depthNativeFormat   = 0;
            XRSwapChainDescriptor           depthEffectiveDesc;
            std::vector<XRSwapchainImage>   depthImages;
            if (CreateNativeSwapChain(depthDesc, depthSwapchain, depthNativeFormat, depthEffectiveDesc, depthImages))
            {
                std::unique_ptr<OpenXRSwapChain> depthCompanion{ new OpenXRSwapChain(
                    binding_, renderSystem_, depthSwapchain, depthEffectiveDesc, depthNativeFormat, std::move(depthImages)) };
                swapChain->AttachDepthCompanion(std::move(depthCompanion));
                depthReady = true;
            }
        }

        // Otherwise fall back to a private depth texture (depth is not submitted to the runtime for reprojection).
        if (!depthReady)
        {
            TextureDescriptor depthTexDesc;
            depthTexDesc.type       = TextureType::Texture2D;
            depthTexDesc.bindFlags  = BindFlags::DepthStencilAttachment;
            depthTexDesc.format     = swapChainDesc.depthStencilFormat;
            depthTexDesc.extent     = { swapChainDesc.resolution.width, swapChainDesc.resolution.height, 1u };
            depthTexDesc.mipLevels  = 1;

            if (Texture* depthTexture = renderSystem_.CreateTexture(depthTexDesc))
                swapChain->AttachDepthTexture(depthTexture);
            else
            {
                report_.Errorf("XRSession::CreateSwapChain failed: could not create fallback depth texture\n");
                ownedSwapChains_.erase(swapChain);
                return nullptr;
            }
        }
    }

    // 3. Build the render targets that wrap the color images (paired with the managed depth, if any).
    if (!swapChain->BuildRenderTargets())
    {
        report_.Errorf("XRSession::CreateSwapChain failed: could not create render targets\n");
        ownedSwapChains_.erase(swapChain);
        return nullptr;
    }

    return swapChain;
}

void OpenXRSession::Release(XRSwapChain& swapChain)
{
    ownedSwapChains_.erase(&swapChain);
}

bool OpenXRSession::IsRunning() const
{
    return running_;
}

bool OpenXRSession::WaitFrame(XRFrameState &frameState)
{
    if (!running_)
        return false;

    XrFrameWaitInfo waitInfo{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState xrFrame{XR_TYPE_FRAME_STATE};
    XrResult result = xrWaitFrame(session_, &waitInfo, &xrFrame);
    if (Failed(result))
    {
        ReportXrError(&report_, XR_NULL_HANDLE, result, "xrWaitFrame");
        return false;
    }

    currentFrameState_ = xrFrame;

    frameState.predictedDisplayTimeNs = xrFrame.predictedDisplayTime;
    frameState.shouldRender = xrFrame.shouldRender;

    return true;
}

bool OpenXRSession::BeginFrame()
{
    if (!running_)
        return false;

    XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
    XrResult result = xrBeginFrame(session_, &beginInfo);
    if (Failed(result) && result != XR_FRAME_DISCARDED)
    {
        ReportXrError(&report_, XR_NULL_HANDLE, result, "xrBeginFrame");
        return false;
    }

    frameStarted_ = true;

    return true;
}

bool OpenXRSession::EndFrame(float nearZ, float farZ, ArrayView<XRSwapChain *> swapChains)
{
    if (!frameStarted_)
        return false;

    XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
    endInfo.displayTime = currentFrameState_.predictedDisplayTime;
    endInfo.environmentBlendMode = environmentBlendMode_;

    XrCompositionLayerProjection projectionLayer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
    SmallVector<XrCompositionLayerProjectionView> projectionViews;
    // Parallel array of depth-info structs; one per view. Lives in a separate array so each
    // projection view's `next` chain entry can point to a stable address.
    SmallVector<XrCompositionLayerDepthInfoKHR> depthInfos;
    const XrCompositionLayerBaseHeader* layers[1] = { nullptr };

    if (currentFrameState_.shouldRender && swapChains.size() == viewCount_ && currentViews_.size() == viewCount_)
    {
        projectionViews.resize(viewCount_, XrCompositionLayerProjectionView{ XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });
        if (depthSubmissionEnabled_)
            depthInfos.resize(viewCount_, XrCompositionLayerDepthInfoKHR{ XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR });

        bool allValid = true;
        for (std::uint32_t i = 0; i < viewCount_; ++i)
        {
            auto* sc = static_cast<OpenXRSwapChain*>(swapChains[i]);
            if (sc == nullptr)
            {
                allValid = false;
                break;
            }
            const XrView &view = currentViews_[i];
            XrCompositionLayerProjectionView& pv = projectionViews[i];
            pv.pose = view.pose;
            pv.fov  = view.fov;

            pv.subImage.swapchain               = sc->GetSwapchain();
            pv.subImage.imageRect.offset        = { 0, 0 };
            pv.subImage.imageRect.extent.width  = static_cast<std::int32_t>(sc->GetResolution().width);
            pv.subImage.imageRect.extent.height = static_cast<std::int32_t>(sc->GetResolution().height);
            pv.subImage.imageArrayIndex         = 0;

            // If the runtime supports composition_layer_depth and the application paired a depth
            // swap-chain with this color swap-chain, chain XrCompositionLayerDepthInfoKHR.
            if (depthSubmissionEnabled_)
            {
                if (auto* depthCompanion = sc->GetDepthCompanion())
                {
                    XrCompositionLayerDepthInfoKHR& di = depthInfos[i];
                    di.subImage.swapchain               = depthCompanion->GetSwapchain();
                    di.subImage.imageRect.offset        = { 0, 0 };
                    di.subImage.imageRect.extent.width  = static_cast<std::int32_t>(depthCompanion->GetResolution().width);
                    di.subImage.imageRect.extent.height = static_cast<std::int32_t>(depthCompanion->GetResolution().height);
                    di.subImage.imageArrayIndex         = 0;
                    di.minDepth                         = 0.0f;
                    di.maxDepth                         = 1.0f;
                    di.nearZ                            = nearZ;
                    di.farZ                             = farZ;
                    pv.next = &di;
                }
            }
        }

        if (allValid)
        {
            projectionLayer.space      = referenceSpace_;
            projectionLayer.viewCount  = viewCount_;
            projectionLayer.views      = projectionViews.data();
            layers[0] = reinterpret_cast<const XrCompositionLayerBaseHeader*>(&projectionLayer);
            endInfo.layerCount = 1;
            endInfo.layers     = layers;
        }
    }

    // Release every image acquired for this frame back to the runtime before submitting. The application has
    // already submitted its render commands by now, so the released images hold the finished frame.
    for (XRSwapChain* swapChain : swapChains)
    {
        if (auto* sc = static_cast<OpenXRSwapChain*>(swapChain))
            sc->ReleaseImage();
    }

    const XrResult result = xrEndFrame(session_, &endInfo);
    frameStarted_ = false;
    if (Failed(result))
    {
        ReportXrError(&report_, XR_NULL_HANDLE, result, "xrEndFrame");
        return false;
    }

    // Acquire-ahead: immediately acquire and wait for the next frame's image on each swap-chain so the next
    // frame's XRSwapChain::GetRenderTarget is a pure accessor. This mirrors how LLGL::SwapChain::Present
    // acquires its next image at the tail of present, and lets the wait overlap the gap before the next WaitFrame.
    for (XRSwapChain* swapChain : swapChains)
    {
        if (auto* sc = static_cast<OpenXRSwapChain*>(swapChain))
            sc->AcquireNextImage();
    }

    return true;
}

bool OpenXRSession::GetViewState(DynamicVector<XRViewPose> &viewsOut)
{
    // Sample the poses of the views again to get updated estimates for the current display time.
    // The runtime may have updated its internal tracking data since the last xrWaitFrame call,
    // so this can improve the accuracy of the view poses used for rendering and hit-testing.

    XrViewState viewState{XR_TYPE_VIEW_STATE};
    XrViewLocateInfo locateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    locateInfo.viewConfigurationType    = viewConfigurationType_;
    locateInfo.displayTime              = currentFrameState_.predictedDisplayTime;
    locateInfo.space                    = referenceSpace_;

    std::uint32_t actualViews = 0;

    XrResult result = xrLocateViews(
        session_,
        &locateInfo,
        &viewState,
        viewCount_,
        &actualViews,
        currentViews_.data());
    if (Failed(result))
    {
        ReportXrError(&report_, XR_NULL_HANDLE, result, "xrLocateViews");
        return false;
    }

    const bool posesValid =
        (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) != 0 &&
        (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) != 0;

    viewsOut.resize(actualViews);
    for (std::uint32_t i = 0; i < actualViews; ++i)
    {
        const XrView &v = currentViews_[i];
        XRViewPose &outView = viewsOut[i];
        outView.orientation[0] = v.pose.orientation.x;
        outView.orientation[1] = v.pose.orientation.y;
        outView.orientation[2] = v.pose.orientation.z;
        outView.orientation[3] = v.pose.orientation.w;
        outView.position[0] = v.pose.position.x;
        outView.position[1] = v.pose.position.y;
        outView.position[2] = v.pose.position.z;
        outView.fovAngleLeft = v.fov.angleLeft;
        outView.fovAngleRight = v.fov.angleRight;
        outView.fovAngleUp = v.fov.angleUp;
        outView.fovAngleDown = v.fov.angleDown;
    }

    return true;
}

bool OpenXRSession::GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle == nullptr || nativeHandleSize != sizeof(SessionNativeHandle))
        return false;

    auto *handle = reinterpret_cast<SessionNativeHandle*>(nativeHandle);
    handle->session = session_;
    handle->referenceSpace = referenceSpace_;
    return true;
}

const Report* OpenXRSession::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

void OpenXRSession::HandleSessionStateChanged(XrSessionState newState)
{
    state_ = newState;

    switch (newState)
    {
        case XR_SESSION_STATE_READY:
        {
            XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
            beginInfo.primaryViewConfigurationType = viewConfigurationType_;
            const XrResult result = xrBeginSession(session_, &beginInfo);
            if (XR_SUCCEEDED(result))
                running_ = true;
            else
                ReportXrError(&report_, XR_NULL_HANDLE, result, "xrBeginSession");
        }
        break;

        case XR_SESSION_STATE_STOPPING:
        {
            const XrResult result = xrEndSession(session_);
            running_ = false;
            if (Failed(result))
                ReportXrError(&report_, XR_NULL_HANDLE, result, "xrEndSession");
        }
        break;

        case XR_SESSION_STATE_LOSS_PENDING:
        case XR_SESSION_STATE_EXITING:
            running_ = false;
            break;

        default:
            break;
    }
}


} // /namespace OpenXR

} // /namespace LLGL



// ================================================================================
