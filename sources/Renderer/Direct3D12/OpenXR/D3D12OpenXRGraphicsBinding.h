/*
 * D3D12OpenXRGraphicsBinding.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_OPENXR_GRAPHICS_BINDING_H
#define LLGL_D3D12_OPENXR_GRAPHICS_BINDING_H


#ifdef LLGL_BUILD_XR_OPENXR


#include <d3d12.h>
#include <dxgi1_4.h>
#include "../../DXCommon/ComPtr.h"

#ifndef XR_USE_GRAPHICS_API_D3D12
#   define XR_USE_GRAPHICS_API_D3D12
#endif

#include "../../../XR/OpenXR/OpenXRPlatform.h"
#include "../../../XR/OpenXR/OpenXRGraphicsBinding.h"


namespace LLGL
{

namespace OpenXR
{


class D3D12OpenXRGraphicsBinding final : public GraphicsBinding
{

    public:

        D3D12OpenXRGraphicsBinding();
        ~D3D12OpenXRGraphicsBinding() override;

        const char*             GetRendererModuleName() const override;
        ArrayView<const char*>  GetRequiredXrExtensions() const override;

        RenderSystemPtr CreateRenderSystem(
            XrInstance                          instance,
            XrSystemId                          systemId,
            const RenderSystemDescriptor&       renderSystemDesc,
            Report*                             report) override;

        const void*     GetSessionGraphicsBinding(RenderSystem& renderSystem) override;

        Format          SelectColorFormat(
            ArrayView<std::int64_t>     runtimeFormats,
            Format                      preferred,
            std::int64_t&               outNativeFormat) const override;

        Format          SelectDepthFormat(
            ArrayView<std::int64_t>     runtimeFormats,
            Format                      preferred,
            std::int64_t&               outNativeFormat) const override;

        bool            EnumerateSwapchainImages(
            RenderSystem&                       renderSystem,
            XrSwapchain                         swapchain,
            const XRSwapChainDescriptor&        swapChainDesc,
            std::int64_t                        nativeFormat,
            SwapchainKind                       kind,
            std::vector<XRSwapchainImage>&      outImages,
            Report*                             report) override;

    private:

        XrGraphicsBindingD3D12KHR   graphicsBinding_    { XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };

};


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR


#endif



// ================================================================================
