/*
 * D3D11OpenXRGraphicsBinding.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef LLGL_BUILD_XR_OPENXR


#include "D3D11OpenXRGraphicsBinding.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>
#include <LLGL/Report.h>

#include "../../../XR/OpenXR/OpenXRError.h"
#include "../../../Core/MacroUtils.h"
#include "../../DXCommon/DXTypes.h"
#include "../Texture/D3D11Texture.h"

#include <vector>


namespace LLGL
{

namespace OpenXR
{

static const char* g_requiredExtensions[] =
{
    XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
};


static bool GetD3D11NativeHandle(RenderSystem& renderSystem, Direct3D11::RenderSystemNativeHandle& outHandle)
{
    return renderSystem.GetNativeHandle(&outHandle, sizeof(outHandle));
}


D3D11OpenXRGraphicsBinding::D3D11OpenXRGraphicsBinding() = default;
D3D11OpenXRGraphicsBinding::~D3D11OpenXRGraphicsBinding() = default;

const char* D3D11OpenXRGraphicsBinding::GetRendererModuleName() const
{
    return "Direct3D11";
}

ArrayView<const char*> D3D11OpenXRGraphicsBinding::GetRequiredXrExtensions() const
{
    return ArrayView<const char*>{ g_requiredExtensions, LLGL_ARRAY_LENGTH(g_requiredExtensions) };
}

RenderSystemPtr D3D11OpenXRGraphicsBinding::CreateRenderSystem(
    XrInstance                          instance,
    XrSystemId                          systemId,
    const RenderSystemDescriptor&       renderSystemDesc,
    Report*                             report)
{
    // Query the runtime's adapter-LUID + minimum-feature-level constraints and pass them down
    // into the backend; the D3D11 backend handles all actual device creation (factory choice,
    // debug flag, software fallback, feature-level ladder, videoAdapterInfo_ population) and
    // honors the LUID + minFL we provide via the native-handle descriptor.
    PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetReq = nullptr;
    XrResult xrResult = xrGetInstanceProcAddr(
        instance, "xrGetD3D11GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetReq));
    if (Failed(xrResult) || pfnGetReq == nullptr)
    {
        ReportXrError(report, instance, xrResult, "xrGetInstanceProcAddr(\"xrGetD3D11GraphicsRequirementsKHR\")");
        return nullptr;
    }

    XrGraphicsRequirementsD3D11KHR gfxReq{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
    xrResult = pfnGetReq(instance, systemId, &gfxReq);
    if (Failed(xrResult))
    {
        ReportXrError(report, instance, xrResult, "xrGetD3D11GraphicsRequirementsKHR");
        return nullptr;
    }

    Direct3D11::RenderSystemNativeHandle nativeHandle{};
    nativeHandle.preferredAdapterLuid = gfxReq.adapterLuid;
    nativeHandle.minFeatureLevel      = gfxReq.minFeatureLevel;

    RenderSystemDescriptor rsDesc = renderSystemDesc;
    rsDesc.nativeHandle     = &nativeHandle;
    rsDesc.nativeHandleSize = sizeof(nativeHandle);

    return RenderSystem::Load(rsDesc, report);
}

const void* D3D11OpenXRGraphicsBinding::GetSessionGraphicsBinding(RenderSystem& renderSystem)
{
    Direct3D11::RenderSystemNativeHandle native{};
    if (!GetD3D11NativeHandle(renderSystem, native))
        return nullptr;
    // GetNativeHandle AddRef's the COM pointers; release the one we don't keep.
    if (native.deviceContext)
        native.deviceContext->Release();
    if (native.device == nullptr)
        return nullptr;

    graphicsBinding_        = XrGraphicsBindingD3D11KHR{ XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
    graphicsBinding_.device = native.device;
    // We don't release native.device here: graphicsBinding_ holds the bare pointer and the
    // runtime expects it to remain valid until xrCreateSession returns. After xrCreateSession,
    // we release it (handled below via a ComPtr swap).
    if (device_)
        device_->Release(); // drop our previous ref
    device_.Attach(native.device); // adopt without AddRef'ing again
    return &graphicsBinding_;
}

// Find the first runtime DXGI format matching the requested color/depth bucket and (optionally)
// LLGL Format. Implemented as one iteration that Unmaps each runtime format down to its canonical
// LLGL Format and compares — this is robust against LLGL's habit of routing depth formats through
// typeless DXGI values (Format::D32Float → R32_TYPELESS via ToDXGIFormat, whereas OpenXR runtimes
// enumerate actual D32_FLOAT). It also handles the color/depth bucketing the runtime doesn't —
// xrEnumerateSwapchainFormats returns both kinds in one mixed list.
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

Format D3D11OpenXRGraphicsBinding::SelectColorFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ false, outNativeFormat);
}

Format D3D11OpenXRGraphicsBinding::SelectDepthFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ true, outNativeFormat);
}

bool D3D11OpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        nativeFormat,
    SwapchainKind                       kind,
    std::vector<XRSwapchainImage>&      outImages,
    Report*                             report)
{
    Direct3D11::RenderSystemNativeHandle native{};
    if (!GetD3D11NativeHandle(renderSystem, native))
    {
        if (report) report->Errorf("D3D11OpenXRGraphicsBinding::EnumerateSwapchainImages failed: render system did not yield a D3D11 native handle\n");
        return false;
    }
    // Release the device-context ref AddRef'd by GetNativeHandle; we only need the device.
    if (native.deviceContext)
        native.deviceContext->Release();

    std::uint32_t imageCount = 0;
    XrResult result = xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr);
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateSwapchainImages");
        if (native.device) native.device->Release();
        return false;
    }

    std::vector<XrSwapchainImageD3D11KHR> xrImages(imageCount, XrSwapchainImageD3D11KHR{ XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
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

    const long bindFlags = (kind == SwapchainKind::DepthStencil)
                               ? BindFlags::DepthStencilAttachment
                               : (BindFlags::ColorAttachment | BindFlags::Sampled);

    for (auto& xrImg : xrImages)
    {
        XRSwapchainImage entry;
        entry.texture = std::unique_ptr<D3D11Texture>(new D3D11Texture(native.device, TextureType::Texture2D, bindFlags, xrImg.texture, swapChainDesc.format));
        outImages.emplace_back(std::move(entry));
    }
    if (native.device) native.device->Release();
    return true;
}


void D3D11OpenXRGraphicsBinding::FlushPendingGpuWork(RenderSystem& renderSystem)
{
    // Force the immediate context to submit pending commands so the OpenXR runtime sees the
    // rendered image when it begins compositing — D3D11 has no implicit submission and the
    // runtime is otherwise free to sample stale contents.
    Direct3D11::RenderSystemNativeHandle native{};
    if (!GetD3D11NativeHandle(renderSystem, native))
        return;
    if (native.device)
        native.device->Release();
    if (native.deviceContext != nullptr)
    {
        native.deviceContext->Flush();
        native.deviceContext->Release();
    }
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D11GraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new D3D11OpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
