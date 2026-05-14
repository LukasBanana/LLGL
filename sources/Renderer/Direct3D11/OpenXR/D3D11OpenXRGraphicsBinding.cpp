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
#include "../Texture/D3D11Texture.h"

#include <dxgi1_2.h>
#include <vector>


namespace LLGL
{

namespace OpenXR
{


static const char* g_requiredExtensions[] =
{
    XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
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
    return ArrayView<const char*>{ g_requiredExtensions, sizeof(g_requiredExtensions) / sizeof(g_requiredExtensions[0]) };
}

RenderSystemPtr D3D11OpenXRGraphicsBinding::CreateRenderSystem(
    XrInstance                          instance,
    XrSystemId                          systemId,
    const XRRenderSystemDescriptor&     renderSystemDesc,
    Report*                             report)
{
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

    // Find the IDXGIAdapter matching the LUID the runtime requires.
    ComPtr<IDXGIFactory1> factory;
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
        if (report) report->Errorf("xrGetD3D11GraphicsRequirementsKHR returned an LUID not matched by any installed DXGI adapter\n");
        return nullptr;
    }

    // Create the D3D11 device on that adapter at or above the required feature level.
    UINT createFlags = 0;
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dContext;
    D3D_FEATURE_LEVEL chosenLevel = D3D_FEATURE_LEVEL_11_0;
    hr = D3D11CreateDevice(
        adapter.Get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        createFlags,
        featureLevels,
        sizeof(featureLevels) / sizeof(featureLevels[0]),
        D3D11_SDK_VERSION,
        d3dDevice.ReleaseAndGetAddressOf(),
        &chosenLevel,
        d3dContext.ReleaseAndGetAddressOf()
    );
    if (FAILED(hr))
    {
        if (report) report->Errorf("D3D11CreateDevice failed (0x%08X)\n", static_cast<unsigned>(hr));
        return nullptr;
    }

    if (chosenLevel < gfxReq.minFeatureLevel)
    {
        if (report) report->Errorf("D3D11 device feature level (0x%X) is below the runtime's minimum (0x%X)\n",
            static_cast<unsigned>(chosenLevel), static_cast<unsigned>(gfxReq.minFeatureLevel));
        return nullptr;
    }

    // Hand the prepared device to LLGL's D3D11 backend via the existing nativeHandle path.
    Direct3D11::RenderSystemNativeHandle nativeHandle{};
    nativeHandle.device         = d3dDevice.Get();
    nativeHandle.deviceContext  = d3dContext.Get();

    RenderSystemDescriptor rsDesc;
    rsDesc.moduleName           = "Direct3D11";
    rsDesc.flags                = renderSystemDesc.flags;
    rsDesc.rendererConfig       = renderSystemDesc.rendererConfig;
    rsDesc.rendererConfigSize   = renderSystemDesc.rendererConfigSize;
    rsDesc.nativeHandle         = &nativeHandle;
    rsDesc.nativeHandleSize     = sizeof(nativeHandle);
    rsDesc.debugger             = renderSystemDesc.debugger;

    RenderSystemPtr renderSystem = RenderSystem::Load(rsDesc, report);
    if (!renderSystem)
        return nullptr;

    // Hold our own ref to the device so it outlives the render system; the binding sets up
    // XrGraphicsBindingD3D11KHR on every CreateSession() and the runtime keeps a reference
    // for the session's lifetime, but we want the device to remain valid for swap-chain
    // image enumeration even between renderer creation and session creation.
    device_ = d3dDevice;
    return renderSystem;
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

Format D3D11OpenXRGraphicsBinding::SelectColorFormat(
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

Format D3D11OpenXRGraphicsBinding::SelectDepthFormat(
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

bool D3D11OpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        nativeFormat,
    SwapchainKind                       kind,
    SmallVector<SwapchainImage>&        outImages,
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

    D3D11Texture::AdoptionParams params{};
    params.type         = TextureType::Texture2D;
    params.bindFlags    = (kind == SwapchainKind::DepthStencil)
                            ? BindFlags::DepthStencilAttachment
                            : (BindFlags::ColorAttachment | BindFlags::Sampled);
    params.llglFormat   = swapChainDesc.format;
    params.dxgiFormat   = static_cast<DXGI_FORMAT>(nativeFormat);
    params.width        = swapChainDesc.resolution.width;
    params.height       = swapChainDesc.resolution.height;
    params.arraySize    = swapChainDesc.arrayLayers;
    params.mipLevels    = 1;

    for (auto& xrImg : xrImages)
    {
        params.texture = xrImg.texture;
        auto* texture = new D3D11Texture(native.device, params);
        SwapchainImage entry;
        entry.texture = texture;
        outImages.push_back(entry);
    }
    if (native.device) native.device->Release();
    return true;
}

void D3D11OpenXRGraphicsBinding::ReleaseSwapchainImage(RenderSystem& /*renderSystem*/, Texture& texture)
{
    delete static_cast<D3D11Texture*>(&texture);
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D11GraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new D3D11OpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
