/*
 * VKOpenXRGraphicsBinding.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_OPENXR_GRAPHICS_BINDING_H
#define LLGL_VK_OPENXR_GRAPHICS_BINDING_H


#ifdef LLGL_BUILD_XR_OPENXR


// Set up the Vulkan-flavored OpenXR opt-in BEFORE including OpenXRPlatform.h, which
// is the first place openxr_platform.h gets pulled in. Header guards inside the SDK
// only honor XR_USE_* macros set before its first inclusion.
#include <vulkan/vulkan.h>
#ifndef XR_USE_GRAPHICS_API_VULKAN
#   define XR_USE_GRAPHICS_API_VULKAN
#endif

#include "../../../XR/OpenXR/OpenXRPlatform.h"
#include "../../../XR/OpenXR/OpenXRGraphicsBinding.h"


namespace LLGL
{

namespace OpenXR
{


class VKOpenXRGraphicsBinding final : public GraphicsBinding
{

    public:

        VKOpenXRGraphicsBinding();
        ~VKOpenXRGraphicsBinding() override;

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

        // Per-instance cached extension function pointers.
        struct Functions
        {
            PFN_xrGetVulkanGraphicsRequirements2KHR getGraphicsRequirements = nullptr;
            PFN_xrCreateVulkanInstanceKHR           createInstance          = nullptr;
            PFN_xrGetVulkanGraphicsDevice2KHR       getGraphicsDevice       = nullptr;
            PFN_xrCreateVulkanDeviceKHR             createDevice            = nullptr;
        };

        bool LoadXRProc(XrInstance instance, Functions &fns, Report *report);

    private:

        // Vulkan objects we own on behalf of the runtime + LLGL backend.
        // The LLGL Vulkan backend stores only weak references to instance/device when handed in
        // via the nativeHandle path, so this binding is responsible for destroying them.
        VkInstance                  ownedInstance_      = VK_NULL_HANDLE;
        VkDevice                    ownedDevice_        = VK_NULL_HANDLE;

        XrGraphicsBindingVulkanKHR  graphicsBinding_    { XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };

};


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR


#endif



// ================================================================================
