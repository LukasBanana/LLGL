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
#include "VKCore.h"
#include "VKPtr.h"
#include <memory>
#include <vector>


namespace LLGL
{


class VKCommandBuffer;

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

        /* --- Extended functions --- */

        void SetPresentCommandBuffer(VKCommandBuffer* commandBuffer);

        inline const VKPtr<VkDevice>& GetLogicalDevice() const
        {
            return device_;
        }

        inline const VKPtr<VkRenderPass>& GetSwapChainRenderPass() const
        {
            return swapChainRenderPass_;
        }

        inline const QueueFamilyIndices& GetQueueFamilyIndices() const
        {
            return queueFamilyIndices_;
        }

        inline size_t GetSwapChainSize() const
        {
            return swapChainImages_.size();
        }

    private:

        void CreateVkSurface();
        void CreateLogicalDevice();
        void CreateSwapChain(const VideoModeDescriptor& desc);
        void CreateSwapChainImageViews();
        void CreateSwapChainRenderPass();
        void CreateSwapChainFramebuffers();
        void CreateVkSemaphore(VKPtr<VkSemaphore>& semaphore);
        void CreatePresentSemaphorse();

        std::vector<VkQueueFamilyProperties> QueryQueueFamilyProperties(VkPhysicalDevice device);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, const VkQueueFlags flags);

        VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
        VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
        VkExtent2D PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, uint32_t width, uint32_t height) const;

        /* ----- Common objects ----- */

        VkInstance                          instance_                   = VK_NULL_HANDLE;
        VkPhysicalDevice                    physicalDevice_             = VK_NULL_HANDLE;
        VKPtr<VkDevice>                     device_;
        VKPtr<VkSurfaceKHR>                 surface_;
        QueueFamilyIndices                  queueFamilyIndices_;

        VKPtr<VkSwapchainKHR>               swapChain_;
        VKPtr<VkRenderPass>                 swapChainRenderPass_;
        VkFormat                            swapChainFormat_            = VK_FORMAT_UNDEFINED;
        VkExtent2D                          swapChainExtent_            = { 0, 0 };
        std::vector<VkImage>                swapChainImages_;
        std::vector<VKPtr<VkImageView>>     swapChainImageViews_;
        std::vector<VKPtr<VkFramebuffer>>   swapChainFramebuffers_;

        VkQueue                             graphicsQueue_              = VK_NULL_HANDLE;
        VkQueue                             presentQueue_               = VK_NULL_HANDLE;

        VKPtr<VkSemaphore>                  imageAvailableSemaphore_;
        VKPtr<VkSemaphore>                  renderFinishedSemaphore_;

        VKCommandBuffer*                    commandBuffer_              = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
