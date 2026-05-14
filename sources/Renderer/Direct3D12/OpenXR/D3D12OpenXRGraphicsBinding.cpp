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

static Format DxgiColorFormatToLLGL(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_B8G8R8A8_UNORM:        return Format::BGRA8UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return Format::BGRA8UNorm_sRGB;
        case DXGI_FORMAT_R8G8B8A8_UNORM:        return Format::RGBA8UNorm;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return Format::RGBA8UNorm_sRGB;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:    return Format::RGBA16Float;
        default:                                return Format::Undefined;
    }
}

static Format DxgiDepthFormatToLLGL(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_D16_UNORM:             return Format::D16UNorm;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:     return Format::D24UNormS8UInt;
        case DXGI_FORMAT_D32_FLOAT:             return Format::D32Float;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:  return Format::D32FloatS8X24UInt;
        default:                                return Format::Undefined;
    }
}

static DXGI_FORMAT LLGLToDxgiFormat(Format format)
{
    switch (format)
    {
        case Format::BGRA8UNorm:            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8UNorm_sRGB:       return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case Format::RGBA8UNorm:            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8UNorm_sRGB:       return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case Format::RGBA16Float:           return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::D16UNorm:              return DXGI_FORMAT_D16_UNORM;
        case Format::D24UNormS8UInt:        return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32Float:              return DXGI_FORMAT_D32_FLOAT;
        case Format::D32FloatS8X24UInt:     return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:                            return DXGI_FORMAT_UNKNOWN;
    }
}

static bool GetD3D12NativeHandle(RenderSystem& renderSystem, Direct3D12::RenderSystemNativeHandle& outHandle)
{
    return renderSystem.GetNativeHandle(&outHandle, sizeof(outHandle));
}


D3D12OpenXRGraphicsBinding::D3D12OpenXRGraphicsBinding() = default;
D3D12OpenXRGraphicsBinding::~D3D12OpenXRGraphicsBinding() = default;

const char* D3D12OpenXRGraphicsBinding::GetRendererModuleName() const
{
    return "Direct3D12";
}

ArrayView<const char*> D3D12OpenXRGraphicsBinding::GetRequiredXrExtensions() const
{
    return ArrayView<const char*>{ g_requiredExtensions, sizeof(g_requiredExtensions) / sizeof(g_requiredExtensions[0]) };
}

RenderSystemPtr D3D12OpenXRGraphicsBinding::CreateRenderSystem(
    XrInstance                          instance,
    XrSystemId                          systemId,
    const XRRenderSystemDescriptor&     renderSystemDesc,
    Report*                             report)
{
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

    // Find the IDXGIAdapter1 matching the LUID the runtime requires.
    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        if (report) report->Errorf("CreateDXGIFactory1 failed (0x%08X)\n", static_cast<unsigned>(hr));
        return nullptr;
    }

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; ; ++i)
    {
        ComPtr<IDXGIAdapter1> candidate;
        if (factory->EnumAdapters1(i, &candidate) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC1 desc{};
        if (SUCCEEDED(candidate->GetDesc1(&desc)))
        {
            if (desc.AdapterLuid.LowPart  == gfxReq.adapterLuid.LowPart &&
                desc.AdapterLuid.HighPart == gfxReq.adapterLuid.HighPart)
            {
                adapter = candidate;
                break;
            }
        }
    }
    if (!adapter)
    {
        if (report) report->Errorf("xrGetD3D12GraphicsRequirementsKHR returned an LUID not matched by any installed DXGI adapter\n");
        return nullptr;
    }

    ComPtr<ID3D12Device> d3dDevice;
    hr = D3D12CreateDevice(adapter.Get(), gfxReq.minFeatureLevel, IID_PPV_ARGS(&d3dDevice));
    if (FAILED(hr))
    {
        if (report) report->Errorf("D3D12CreateDevice failed (0x%08X) at feature level 0x%X\n",
            static_cast<unsigned>(hr), static_cast<unsigned>(gfxReq.minFeatureLevel));
        return nullptr;
    }

    // Create a graphics command queue. The OpenXR session needs this for synchronization
    // (XrGraphicsBindingD3D12KHR) and LLGL's D3D12 backend will adopt it via nativeHandle.
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ComPtr<ID3D12CommandQueue> d3dQueue;
    hr = d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3dQueue));
    if (FAILED(hr))
    {
        if (report) report->Errorf("ID3D12Device::CreateCommandQueue failed (0x%08X)\n", static_cast<unsigned>(hr));
        return nullptr;
    }

    Direct3D12::RenderSystemNativeHandle nativeHandle{};
    nativeHandle.factory      = factory.Get();
    nativeHandle.device       = d3dDevice.Get();
    nativeHandle.commandQueue = d3dQueue.Get();

    RenderSystemDescriptor rsDesc;
    rsDesc.moduleName           = "Direct3D12";
    rsDesc.flags                = renderSystemDesc.flags;
    rsDesc.rendererConfig       = renderSystemDesc.rendererConfig;
    rsDesc.rendererConfigSize   = renderSystemDesc.rendererConfigSize;
    rsDesc.nativeHandle         = &nativeHandle;
    rsDesc.nativeHandleSize     = sizeof(nativeHandle);

    RenderSystemPtr renderSystem = RenderSystem::Load(rsDesc, report);
    if (!renderSystem)
        return nullptr;

    return renderSystem;
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
    if (preferred != Format::Undefined)
    {
        const DXGI_FORMAT preferredDxgi = LLGLToDxgiFormat(preferred);
        if (preferredDxgi == DXGI_FORMAT_UNKNOWN || DxgiColorFormatToLLGL(preferredDxgi) == Format::Undefined)
        {
            outNativeFormat = 0;
            return Format::Undefined;
        }
        for (std::int64_t fmt : runtimeFormats)
        {
            if (static_cast<DXGI_FORMAT>(fmt) == preferredDxgi)
            {
                outNativeFormat = fmt;
                return preferred;
            }
        }
        outNativeFormat = 0;
        return Format::Undefined;
    }

    for (std::int64_t fmt : runtimeFormats)
    {
        const Format llglFmt = DxgiColorFormatToLLGL(static_cast<DXGI_FORMAT>(fmt));
        if (llglFmt != Format::Undefined)
        {
            outNativeFormat = fmt;
            return llglFmt;
        }
    }

    outNativeFormat = 0;
    return Format::Undefined;
}

Format D3D12OpenXRGraphicsBinding::SelectDepthFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    if (preferred != Format::Undefined)
    {
        const DXGI_FORMAT preferredDxgi = LLGLToDxgiFormat(preferred);
        if (preferredDxgi == DXGI_FORMAT_UNKNOWN || DxgiDepthFormatToLLGL(preferredDxgi) == Format::Undefined)
        {
            outNativeFormat = 0;
            return Format::Undefined;
        }
        for (std::int64_t fmt : runtimeFormats)
        {
            if (static_cast<DXGI_FORMAT>(fmt) == preferredDxgi)
            {
                outNativeFormat = fmt;
                return preferred;
            }
        }
        outNativeFormat = 0;
        return Format::Undefined;
    }

    for (std::int64_t fmt : runtimeFormats)
    {
        const Format llglFmt = DxgiDepthFormatToLLGL(static_cast<DXGI_FORMAT>(fmt));
        if (llglFmt != Format::Undefined)
        {
            outNativeFormat = fmt;
            return llglFmt;
        }
    }

    outNativeFormat = 0;
    return Format::Undefined;
}

bool D3D12OpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        /*nativeFormat*/,
    SwapchainKind                       kind,
    SmallVector<SwapchainImage>&        outImages,
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

    D3D12Texture::AdoptionParams params{};
    params.type         = TextureType::Texture2D;
    params.llglFormat   = swapChainDesc.format;
    if (kind == SwapchainKind::DepthStencil)
    {
        params.bindFlags    = BindFlags::DepthStencilAttachment;
        // Per the OpenXR spec, depth swap-chain images are handed to the application in
        // D3D12_RESOURCE_STATE_DEPTH_WRITE and must be returned in the same state.
        params.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else
    {
        params.bindFlags    = BindFlags::ColorAttachment | BindFlags::Sampled;
        // Per the OpenXR spec, color swap-chain images are handed to the application in
        // D3D12_RESOURCE_STATE_RENDER_TARGET and must be returned in the same state.
        params.initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    for (auto& xrImg : xrImages)
    {
        params.resource = xrImg.texture;
        auto* texture = new D3D12Texture(native.device, params);
        SwapchainImage entry;
        entry.texture = texture;
        outImages.push_back(entry);
    }
    if (native.device) native.device->Release();
    return true;
}

void D3D12OpenXRGraphicsBinding::ReleaseSwapchainImage(RenderSystem& /*renderSystem*/, Texture& texture)
{
    delete static_cast<D3D12Texture*>(&texture);
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D12GraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new D3D12OpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
