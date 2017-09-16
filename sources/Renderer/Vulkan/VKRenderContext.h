/*
 * VKRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_CONTEXT_H
#define LLGL_VK_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <memory>
#include <vector>
#include "VKCore.h"
#include "VKPtr.h"


namespace LLGL
{


class VKRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        VKRenderContext(
            const VKPtr<VkInstance>& instance,
            VkPhysicalDevice physicalDevice,
            RenderContextDescriptor desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

    private:

        void CreateVkSurface();
        void CreateLogicalDevice();
        void CreateSwapChain(const VideoModeDescriptor& desc);
        void CreateSwapChainImageViews();
        void CreateSwapChainRenderPass();
        void CreateSwapChainFramebuffers();

        std::vector<VkQueueFamilyProperties> QueryQueueFamilyProperties(VkPhysicalDevice device);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, const VkQueueFlags flags);

        VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
        VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
        VkExtent2D PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, std::uint32_t width, std::uint32_t height) const;

        /* ----- Common objects ----- */

        VkInstance                          instance_           = VK_NULL_HANDLE;
        VkPhysicalDevice                    physicalDevice_     = VK_NULL_HANDLE;
        VKPtr<VkDevice>                     device_;
        VKPtr<VkSurfaceKHR>                 surface_;

        VKPtr<VkSwapchainKHR>               swapChain_;
        VKPtr<VkRenderPass>                 swapChainRenderPass_;
        VkFormat                            swapChainFormat_    = VK_FORMAT_UNDEFINED;
        VkExtent2D                          swapChainExtent_    = { 0, 0 };
        std::vector<VkImage>                swapChainImages_;
        std::vector<VKPtr<VkImageView>>     swapChainImageViews_;
        std::vector<VKPtr<VkFramebuffer>>   swapChainFramebuffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
