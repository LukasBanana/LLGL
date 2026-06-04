/*
 * D3D12OpenXRGraphicsBinding.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef LLGL_BUILD_XR_OPENXR


#include "D3D12OpenXRGraphicsBinding.h"

#include <LLGL/Backend/Direct3D12/NativeHandle.h>
#include <LLGL/Report.h>

#include "../../../XR/OpenXR/OpenXRError.h"
#include "../../../Core/MacroUtils.h"
#include "../../DXCommon/DXTypes.h"
#include "../Texture/D3D12Texture.h"

#include <vector>


namespace LLGL
{

namespace OpenXR
{


static const char* g_requiredExtensions[] =
{
    XR_KHR_D3D12_ENABLE_EXTENSION_NAME,
};


static bool GetD3D12NativeHandle(RenderSystem& renderSystem, Direct3D12::RenderSystemNativeHandle& outHandle)
{
    return renderSystem.GetNativeHandle(&outHandle, sizeof(outHandle));
}

// Find the first runtime DXGI format matching the requested color/depth bucket and (optionally)
// LLGL Format. See the D3D11 binding's equivalent for the rationale around DXTypes::Unmap vs
// per-binding switch-case lookups and the depth/color bucketing filter via IsDepthStencilDXGIFormat.
static Format FindRuntimeFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    bool                        wantDepth,
    std::int64_t&               outNativeFormat)
{
    for (std::int64_t fmt : runtimeFormats)
    {
        const DXGI_FORMAT dxgiFmt = static_cast<DXGI_FORMAT>(fmt);
        if (DXTypes::IsDepthStencilDXGIFormat(dxgiFmt) != wantDepth)
            continue;
        const Format llglFmt = DXTypes::Unmap(dxgiFmt);
        if (llglFmt == Format::Undefined)
            continue;
        if (preferred != Format::Undefined && llglFmt != preferred)
            continue;
        outNativeFormat = fmt;
        return llglFmt;
    }
    outNativeFormat = 0;
    return Format::Undefined;
}


D3D12OpenXRGraphicsBinding::D3D12OpenXRGraphicsBinding() = default;
D3D12OpenXRGraphicsBinding::~D3D12OpenXRGraphicsBinding() = default;

const char* D3D12OpenXRGraphicsBinding::GetRendererModuleName() const
{
    return "Direct3D12";
}

ArrayView<const char*> D3D12OpenXRGraphicsBinding::GetRequiredXrExtensions() const
{
    return ArrayView<const char*>{ g_requiredExtensions, LLGL_ARRAY_LENGTH(g_requiredExtensions) };
}

RenderSystemPtr D3D12OpenXRGraphicsBinding::CreateRenderSystem(
    XrInstance                          instance,
    XrSystemId                          systemId,
    const RenderSystemDescriptor&       renderSystemDesc,
    Report*                             report)
{
    // Query the runtime's adapter-LUID + minimum-feature-level constraints and pass them down
    // into the backend; the D3D12 backend handles all actual device + command-queue creation
    // (including the debug layer, factory-debug flag, feature-level ladder, and WARP fallback)
    // and honors the LUID + minFL we provide via the native-handle descriptor.
    PFN_xrGetD3D12GraphicsRequirementsKHR pfnGetReq = nullptr;
    XrResult xrResult = xrGetInstanceProcAddr(
        instance, "xrGetD3D12GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetReq));
    if (Failed(xrResult) || pfnGetReq == nullptr)
    {
        ReportXrError(report, instance, xrResult, "xrGetInstanceProcAddr(\"xrGetD3D12GraphicsRequirementsKHR\")");
        return nullptr;
    }

    XrGraphicsRequirementsD3D12KHR gfxReq{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
    xrResult = pfnGetReq(instance, systemId, &gfxReq);
    if (Failed(xrResult))
    {
        ReportXrError(report, instance, xrResult, "xrGetD3D12GraphicsRequirementsKHR");
        return nullptr;
    }

    Direct3D12::RenderSystemNativeHandle nativeHandle{};
    nativeHandle.preferredAdapterLuid = gfxReq.adapterLuid;
    nativeHandle.minFeatureLevel      = gfxReq.minFeatureLevel;

    RenderSystemDescriptor rsDesc = renderSystemDesc;
    rsDesc.nativeHandle     = &nativeHandle;
    rsDesc.nativeHandleSize = sizeof(nativeHandle);

    return RenderSystem::Load(rsDesc, report);
}

const void* D3D12OpenXRGraphicsBinding::GetSessionGraphicsBinding(RenderSystem& renderSystem)
{
    Direct3D12::RenderSystemNativeHandle native{};
    if (!GetD3D12NativeHandle(renderSystem, native))
        return nullptr;
    // GetNativeHandle AddRef's COM pointers; release the factory/device/queue refs we don't keep
    // beyond this call. The runtime reads device + queue during xrCreateSession only.
    if (native.factory) native.factory->Release();

    graphicsBinding_         = XrGraphicsBindingD3D12KHR{ XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };
    graphicsBinding_.device  = native.device;
    graphicsBinding_.queue   = native.commandQueue;
    // Bare pointers: release-after-session-create happens implicitly when the binding is reused.
    // Keep one ref live until the next call by deliberately not releasing here.
    return &graphicsBinding_;
}

Format D3D12OpenXRGraphicsBinding::SelectColorFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ false, outNativeFormat);
}

Format D3D12OpenXRGraphicsBinding::SelectDepthFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ true, outNativeFormat);
}

bool D3D12OpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        /*nativeFormat*/,
    SwapchainKind                       kind,
    std::vector<XRSwapchainImage>&      outImages,
    Report*                             report)
{
    Direct3D12::RenderSystemNativeHandle native{};
    if (!GetD3D12NativeHandle(renderSystem, native))
    {
        if (report) report->Errorf("D3D12OpenXRGraphicsBinding::EnumerateSwapchainImages failed: render system did not yield a D3D12 native handle\n");
        return false;
    }
    // Release factory + queue refs from GetNativeHandle; we only need the device for adoption,
    // and the device ref will be released after constructing all the texture wrappers.
    if (native.factory)      native.factory->Release();
    if (native.commandQueue) native.commandQueue->Release();

    std::uint32_t imageCount = 0;
    XrResult result = xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr);
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateSwapchainImages");
        if (native.device) native.device->Release();
        return false;
    }

    std::vector<XrSwapchainImageD3D12KHR> xrImages(imageCount, XrSwapchainImageD3D12KHR{ XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
    result = xrEnumerateSwapchainImages(
        swapchain,
        imageCount,
        &imageCount,
        reinterpret_cast<XrSwapchainImageBaseHeader*>(xrImages.data())
    );
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateSwapchainImages");
        if (native.device) native.device->Release();
        return false;
    }

    outImages.clear();
    outImages.reserve(imageCount);

    long bindFlags;
    D3D12_RESOURCE_STATES initialState;

    if (kind == SwapchainKind::DepthStencil)
    {
        bindFlags    = BindFlags::DepthStencilAttachment;
        // Per the OpenXR spec, depth swap-chain images are handed to the application in
        // D3D12_RESOURCE_STATE_DEPTH_WRITE and must be returned in the same state.
        initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else
    {
        bindFlags    = BindFlags::ColorAttachment | BindFlags::Sampled;
        // Per the OpenXR spec, color swap-chain images are handed to the application in
        // D3D12_RESOURCE_STATE_RENDER_TARGET and must be returned in the same state.
        initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    for (auto& xrImg : xrImages)
    {
        XRSwapchainImage entry;
        entry.texture = std::unique_ptr<D3D12Texture>(new D3D12Texture(native.device, TextureType::Texture2D, bindFlags, initialState, xrImg.texture, swapChainDesc.format));
        outImages.emplace_back(std::move(entry));
    }
    if (native.device) native.device->Release();
    return true;
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D12GraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new D3D12OpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
