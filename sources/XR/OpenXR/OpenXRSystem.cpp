/*
 * OpenXRSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "OpenXRSystem.h"
#include "OpenXRSession.h"
#include "OpenXRGraphicsBinding.h"
#include "OpenXRError.h"

#include <LLGL/Backend/OpenXR/NativeHandle.h>

#include <cstring>
#include <cstdio>


namespace LLGL
{

namespace OpenXR
{


OpenXRSystem::OpenXRSystem() = default;

OpenXRSystem::~OpenXRSystem()
{
    activeSession_.reset();
    if (instance_ != XR_NULL_HANDLE)
        xrDestroyInstance(instance_);
}

const char* OpenXRSystem::GetRuntimeName() const
{
    return runtimeName_;
}

ArrayView<XRViewConfigurationView> OpenXRSystem::GetViewConfigurations() const
{
    return ArrayView<XRViewConfigurationView>{ viewConfigViews_.data(), viewConfigViews_.size() };
}

const Report* OpenXRSystem::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

bool OpenXRSystem::Startup(const XRSystemDescriptor& desc, Report* report)
{
    formFactor_         = desc.formFactor;
    viewConfiguration_  = desc.viewConfiguration;
    viewConfigurationType_ = (desc.viewConfiguration == XRViewConfiguration::Stereo)
        ? XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO
        : XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO;

    Report* outReport = report != nullptr ? report : &report_;

#ifdef LLGL_OS_ANDROID
    if (desc.androidApp == nullptr)
    {
        outReport->Errorf("XRSystem::Load failed on Android: XRSystemDescriptor::androidApp must not be null\n");
        return false;
    }
    androidApp_ = desc.androidApp;

    // The Android OpenXR loader requires a one-time initialization with the JavaVM and Activity
    // before any other XR call. xrInitializeLoaderKHR is loaded with XR_NULL_HANDLE for the instance.
    PFN_xrInitializeLoaderKHR pfnInitializeLoader = nullptr;
    XrResult initLoaderLookup = xrGetInstanceProcAddr(
        XR_NULL_HANDLE, "xrInitializeLoaderKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&pfnInitializeLoader));
    if (Failed(initLoaderLookup) || pfnInitializeLoader == nullptr)
    {
        ReportXrError(outReport, XR_NULL_HANDLE, initLoaderLookup, "xrGetInstanceProcAddr(\"xrInitializeLoaderKHR\")");
        return false;
    }

    XrLoaderInitInfoAndroidKHR loaderInit{ XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
    loaderInit.applicationVM      = androidApp_->activity->vm;
    loaderInit.applicationContext = androidApp_->activity->clazz;
    XrResult initResult = pfnInitializeLoader(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&loaderInit));
    if (Failed(initResult))
    {
        ReportXrError(outReport, XR_NULL_HANDLE, initResult, "xrInitializeLoaderKHR");
        return false;
    }
#endif

    if (!LoadGraphicsBindings(outReport))
        return false;

    if (!CreateInstance(desc, outReport))
        return false;

    if (!QuerySystem(desc, outReport))
        return false;

    if (!QueryViewConfigurations(outReport))
        return false;

    return true;
}

bool OpenXRSystem::LoadGraphicsBindings(Report* /*report*/)
{
#ifdef LLGL_XR_OPENXR_BIND_VULKAN
    bindings_.push_back(CreateVulkanGraphicsBinding());
#endif
#ifdef LLGL_XR_OPENXR_BIND_DIRECT3D11
    bindings_.push_back(CreateD3D11GraphicsBinding());
#endif
#ifdef LLGL_XR_OPENXR_BIND_DIRECT3D12
    bindings_.push_back(CreateD3D12GraphicsBinding());
#endif
    return true;
}

bool OpenXRSystem::CreateInstance(const XRSystemDescriptor& desc, Report* report)
{
    // Collect required extensions across all known graphics bindings, then filter against runtime-available list.
    SmallVector<const char*> requiredExts;
    for (auto& binding : bindings_)
    {
        for (const char* ext : binding->GetRequiredXrExtensions())
            requiredExts.push_back(ext);
    }

    if ((desc.flags & XRSystemFlags::DebugDevice) != 0)
        requiredExts.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // KHR_composition_layer_depth is optional: we request it but the runtime is allowed not to
    // advertise it. If absent, depth swap-chains can still be created and rendered to (so the
    // application keeps its depth buffer), but XRSession::EndFrame won't chain depth info into
    // the projection layer.
    const char* const kOptionalCompositionDepth = XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME;
    requiredExts.push_back(kOptionalCompositionDepth);

#ifdef LLGL_OS_ANDROID
    // Android-specific extensions: the loader-init extension is technically already used
    // (via xrInitializeLoaderKHR above), and KHR_android_create_instance gates the
    // XrInstanceCreateInfoAndroidKHR struct chained into xrCreateInstance below.
    requiredExts.push_back(XR_KHR_LOADER_INIT_ANDROID_EXTENSION_NAME);
    requiredExts.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
#endif

    // Enumerate available extensions.
    std::uint32_t availableExtCount = 0;
    XrResult result = xrEnumerateInstanceExtensionProperties(nullptr, 0, &availableExtCount, nullptr);
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateInstanceExtensionProperties");
        return false;
    }

    SmallVector<XrExtensionProperties> availableExts;
    availableExts.resize(availableExtCount, XrExtensionProperties{ XR_TYPE_EXTENSION_PROPERTIES });
    result = xrEnumerateInstanceExtensionProperties(nullptr, availableExtCount, &availableExtCount, availableExts.data());
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateInstanceExtensionProperties");
        return false;
    }

    auto isExtAvailable = [&](const char* name) -> bool
    {
        for (const auto& prop : availableExts)
        {
            if (std::strcmp(prop.extensionName, name) == 0)
                return true;
        }
        return false;
    };

    SmallVector<const char*> enabledExts;
    for (const char* ext : requiredExts)
    {
        if (isExtAvailable(ext))
        {
            enabledExts.push_back(ext);
            if (std::strcmp(ext, XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME) == 0)
                depthSubmissionEnabled_ = true;
        }
    }

    XrInstanceCreateInfo info{ XR_TYPE_INSTANCE_CREATE_INFO };
    // Target OpenXR 1.0 explicitly: it's what every shipping runtime (SteamVR, WMR, Oculus, Monado)
    // supports today. Using XR_CURRENT_API_VERSION from a 1.1+ SDK would request 1.1 and runtimes
    // that haven't adopted it yet return XR_ERROR_API_VERSION_UNSUPPORTED.
    info.applicationInfo.apiVersion = XR_API_VERSION_1_0;

#ifdef LLGL_OS_ANDROID
    // Android runtimes (Quest, HTC Wave, Pico, etc.) require the JavaVM and Activity to be
    // chained into instance creation via KHR_android_create_instance.
    XrInstanceCreateInfoAndroidKHR androidInfo{ XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
    androidInfo.applicationVM       = androidApp_->activity->vm;
    androidInfo.applicationActivity = androidApp_->activity->clazz;
    info.next = &androidInfo;
#endif
    std::snprintf(info.applicationInfo.applicationName, XR_MAX_APPLICATION_NAME_SIZE, "%s", desc.applicationName.c_str());
    std::snprintf(info.applicationInfo.engineName, XR_MAX_ENGINE_NAME_SIZE, "%s", desc.engineName.c_str());
    info.applicationInfo.applicationVersion = desc.applicationVersion;
    info.applicationInfo.engineVersion      = desc.engineVersion;
    info.enabledExtensionCount              = static_cast<std::uint32_t>(enabledExts.size());
    info.enabledExtensionNames              = enabledExts.empty() ? nullptr : enabledExts.data();

    result = xrCreateInstance(&info, &instance_);
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrCreateInstance");
        return false;
    }

    XrInstanceProperties props{ XR_TYPE_INSTANCE_PROPERTIES };
    if (XR_SUCCEEDED(xrGetInstanceProperties(instance_, &props)))
        std::snprintf(runtimeName_, sizeof(runtimeName_), "%s", props.runtimeName);

    return true;
}

bool OpenXRSystem::QuerySystem(const XRSystemDescriptor& /*desc*/, Report* report)
{
    XrSystemGetInfo info{ XR_TYPE_SYSTEM_GET_INFO };
    info.formFactor = (formFactor_ == XRFormFactor::HeadMountedDisplay)
        ? XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
        : XR_FORM_FACTOR_HANDHELD_DISPLAY;

    const XrResult result = xrGetSystem(instance_, &info, &systemId_);
    if (Failed(result))
    {
        ReportXrError(report, instance_, result, "xrGetSystem");
        return false;
    }
    return true;
}

bool OpenXRSystem::QueryViewConfigurations(Report* report)
{
    std::uint32_t viewCount = 0;
    XrResult result = xrEnumerateViewConfigurationViews(
        instance_, systemId_, viewConfigurationType_, 0, &viewCount, nullptr);
    if (Failed(result))
    {
        ReportXrError(report, instance_, result, "xrEnumerateViewConfigurationViews");
        return false;
    }

    SmallVector<XrViewConfigurationView> xrViews;
    xrViews.resize(viewCount, XrViewConfigurationView{ XR_TYPE_VIEW_CONFIGURATION_VIEW });
    result = xrEnumerateViewConfigurationViews(
        instance_, systemId_, viewConfigurationType_, viewCount, &viewCount, xrViews.data());
    if (Failed(result))
    {
        ReportXrError(report, instance_, result, "xrEnumerateViewConfigurationViews");
        return false;
    }

    viewConfigViews_.resize(viewCount);
    for (std::uint32_t i = 0; i < viewCount; ++i)
    {
        viewConfigViews_[i].recommendedImageWidth  = xrViews[i].recommendedImageRectWidth;
        viewConfigViews_[i].recommendedImageHeight = xrViews[i].recommendedImageRectHeight;
        viewConfigViews_[i].maxImageWidth          = xrViews[i].maxImageRectWidth;
        viewConfigViews_[i].maxImageHeight         = xrViews[i].maxImageRectHeight;
        viewConfigViews_[i].recommendedSampleCount = xrViews[i].recommendedSwapchainSampleCount;
        viewConfigViews_[i].maxSampleCount         = xrViews[i].maxSwapchainSampleCount;
    }

    // Cache supported blend modes for later session creation.
    std::uint32_t blendModeCount = 0;
    result = xrEnumerateEnvironmentBlendModes(
        instance_, systemId_, viewConfigurationType_, 0, &blendModeCount, nullptr);
    if (XR_SUCCEEDED(result) && blendModeCount > 0)
    {
        supportedBlendModes_.resize(blendModeCount);
        xrEnumerateEnvironmentBlendModes(
            instance_, systemId_, viewConfigurationType_, blendModeCount, &blendModeCount, supportedBlendModes_.data());
    }
    return true;
}

GraphicsBinding* OpenXRSystem::FindBindingByName(const char* name) const
{
    for (auto& b : bindings_)
    {
        if (std::strcmp(b->GetRendererModuleName(), name) == 0)
            return b.get();
    }
    return nullptr;
}

XrEnvironmentBlendMode OpenXRSystem::SelectEnvironmentBlendMode(XREnvironmentBlendMode preferred) const
{
    XrEnvironmentBlendMode preferredXr = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    switch (preferred)
    {
        case XREnvironmentBlendMode::Opaque:     preferredXr = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;     break;
        case XREnvironmentBlendMode::Additive:   preferredXr = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;   break;
        case XREnvironmentBlendMode::AlphaBlend: preferredXr = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND;break;
    }
    for (auto mode : supportedBlendModes_)
    {
        if (mode == preferredXr)
            return preferredXr;
    }
    return supportedBlendModes_.empty() ? XR_ENVIRONMENT_BLEND_MODE_OPAQUE : supportedBlendModes_[0];
}

RenderSystemPtr OpenXRSystem::CreateRenderSystem(const XRRenderSystemDescriptor& renderSystemDesc, Report* report)
{
    Report* outReport = report != nullptr ? report : &report_;

    GraphicsBinding* binding = FindBindingByName(renderSystemDesc.rendererModule.c_str());
    if (binding == nullptr)
    {
        outReport->Errorf(
            "XRSystem::CreateRenderSystem failed: no XR graphics binding registered for renderer '%s'\n",
            renderSystemDesc.rendererModule.c_str());
        return nullptr;
    }

    XRRenderSystemDescriptor effectiveDesc = renderSystemDesc;
#ifdef LLGL_OS_ANDROID
    // Auto-forward the android_app captured at Load() time so applications don't need to repeat it.
    // An explicitly-set value on the descriptor wins (lets advanced cases override).
    if (effectiveDesc.androidApp == nullptr)
        effectiveDesc.androidApp = androidApp_;
#endif

    RenderSystemPtr renderSystem = binding->CreateRenderSystem(instance_, systemId_, effectiveDesc, outReport);
    if (renderSystem)
        activeBinding_ = binding;
    return renderSystem;
}

XRSession* OpenXRSystem::CreateSession(const XRSessionDescriptor& sessionDesc, RenderSystem& renderSystem)
{
    if (activeBinding_ == nullptr)
    {
        report_.Errorf("XRSystem::CreateSession failed: no active graphics binding (call CreateRenderSystem first)\n");
        return nullptr;
    }

    const void* graphicsBindingChain = activeBinding_->GetSessionGraphicsBinding(renderSystem);
    if (graphicsBindingChain == nullptr)
    {
        report_.Errorf("XRSystem::CreateSession failed: graphics binding refused to produce a session binding chain\n");
        return nullptr;
    }

    XrSessionCreateInfo info{ XR_TYPE_SESSION_CREATE_INFO };
    info.next      = graphicsBindingChain;
    info.systemId  = systemId_;

    XrSession session = XR_NULL_HANDLE;
    XrResult result = xrCreateSession(instance_, &info, &session);
    if (Failed(result))
    {
        ReportXrError(&report_, instance_, result, "xrCreateSession");
        return nullptr;
    }

    XrReferenceSpaceCreateInfo spaceInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    spaceInfo.poseInReferenceSpace.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };

    XrSpace referenceSpace = XR_NULL_HANDLE;
    result = xrCreateReferenceSpace(session, &spaceInfo, &referenceSpace);
    if (Failed(result))
    {
        ReportXrError(&report_, instance_, result, "xrCreateReferenceSpace");
        xrDestroySession(session);
        return nullptr;
    }

    // Enumerate supported swap-chain formats.
    std::uint32_t formatCount = 0;
    xrEnumerateSwapchainFormats(session, 0, &formatCount, nullptr);
    SmallVector<std::int64_t> runtimeFormats;
    runtimeFormats.resize(formatCount);
    if (formatCount > 0)
        xrEnumerateSwapchainFormats(session, formatCount, &formatCount, runtimeFormats.data());

    auto newSession = std::unique_ptr<OpenXRSession>(new OpenXRSession(
        *this,
        *activeBinding_,
        renderSystem,
        session,
        referenceSpace,
        viewConfigurationType_,
        SelectEnvironmentBlendMode(sessionDesc.environmentBlendMode),
        static_cast<std::uint32_t>(viewConfigViews_.size()),
        std::move(runtimeFormats),
        depthSubmissionEnabled_
    ));

    activeSession_ = std::move(newSession);
    return activeSession_.get();
}

void OpenXRSystem::Release(XRSession& session)
{
    if (activeSession_.get() == &session)
        activeSession_.reset();
}

bool OpenXRSystem::PollEvents()
{
    bool anyHandled = false;
    XrEventDataBuffer event{ XR_TYPE_EVENT_DATA_BUFFER };
    while (xrPollEvent(instance_, &event) == XR_SUCCESS)
    {
        anyHandled = true;
        switch (event.type)
        {
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                const auto& evt = *reinterpret_cast<const XrEventDataSessionStateChanged*>(&event);
                if (activeSession_ && activeSession_->GetSession() == evt.session)
                    activeSession_->HandleSessionStateChanged(evt.state);
            }
            break;

            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
                // Mark session as lost; application should tear down.
                if (activeSession_)
                    activeSession_->HandleSessionStateChanged(XR_SESSION_STATE_LOSS_PENDING);
                break;

            default:
                break;
        }
        event = XrEventDataBuffer{ XR_TYPE_EVENT_DATA_BUFFER };
    }
    return anyHandled;
}

bool OpenXRSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenXR::SystemNativeHandle))
    {
        auto* h = static_cast<OpenXR::SystemNativeHandle*>(nativeHandle);
        h->instance = instance_;
        h->systemId = systemId_;
        return true;
    }
    return false;
}


} // /namespace OpenXR

} // /namespace LLGL



// ================================================================================
