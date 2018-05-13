/*
 * VKRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
class VKDeviceMemoryManager;
class VKDeviceMemoryRegion;

class VKRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        VKRenderContext(
            const VKPtr<VkInstance>& instance,
            VkPhysicalDevice physicalDevice,
            const VKPtr<VkDevice>& device,
            VKDeviceMemoryManager& deviceMemoryMngr,
            RenderContextDescriptor desc,
            const std::shared_ptr<Surface>& surface
        );

        ~VKRenderContext();

        void Present() override;

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        /* --- Extended functions --- */

        void SetPresentCommandBuffer(VKCommandBuffer* commandBuffer);

        inline const VKPtr<VkRenderPass>& GetSwapChainRenderPass() const
        {
            return swapChainRenderPass_;
        }

        // Returns the number of images the swap chain has.
        inline size_t GetSwapChainSize() const
        {
            return swapChainImages_.size();
        }

        // Returns the active VkFramebuffer object from the swap chain.
        inline VkFramebuffer GetSwapChainFramebuffer() const
        {
            return swapChainFramebuffers_[presentImageIndex_].Get();
        }

        // Returns the active VkImage object from the swap chain.
        inline VkImage GetSwapChainImage() const
        {
            return swapChainImages_[presentImageIndex_];
        }

        // Returns the 2D extend (i.e. resolution) of the swap chain.
        inline const VkExtent2D& GetSwapChainExtent() const
        {
            return swapChainExtent_;
        }

        // Returns true if this render context has a depth-stencil buffer.
        bool HasDepthStencilBuffer() const;

    private:

        void CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore);
        void CreatePresentSemaphores();
        void CreateGpuSurface();

        void CreateSwapChainRenderPass();
        void CreateSwapChain(const VideoModeDescriptor& videoModeDesc, const VsyncDescriptor& vsyncDesc);
        void CreateSwapChainImageViews();
        void CreateSwapChainFramebuffers();

        void CreateDepthStencilBuffer(const VideoModeDescriptor& videoModeDesc);
        void CreateDepthStencilImage(const VideoModeDescriptor& videoModeDesc);
        void CreateDepthStencilImageView(const VideoModeDescriptor& videoModeDesc);
        void CreateDepthStencilMemory();
        void ReleaseDepthStencilBuffer();

        VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
        VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, const VsyncDescriptor& vsyncDesc) const;
        VkExtent2D PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, std::uint32_t width, std::uint32_t height) const;
        VkFormat PickDepthStencilFormat() const;
        VkFormat PickDepthFormat() const;

        void AcquireNextPresentImage();

        /* ----- Common objects ----- */

        VkInstance                          instance_                   = VK_NULL_HANDLE;
        VkPhysicalDevice                    physicalDevice_             = VK_NULL_HANDLE;
        const VKPtr<VkDevice>&              device_;

        VKDeviceMemoryManager&              deviceMemoryMngr_;

        VKPtr<VkSurfaceKHR>                 surface_;
        SurfaceSupportDetails               surfaceSupportDetails_;

        VKPtr<VkSwapchainKHR>               swapChain_;
        VKPtr<VkRenderPass>                 swapChainRenderPass_;
        VkSurfaceFormatKHR                  swapChainFormat_;
        VkExtent2D                          swapChainExtent_            = { 0, 0 };
        std::vector<VkImage>                swapChainImages_;
        std::vector<VKPtr<VkImageView>>     swapChainImageViews_;
        std::vector<VKPtr<VkFramebuffer>>   swapChainFramebuffers_;
        std::uint32_t                       presentImageIndex_          = 0;

        VkFormat                            depthStencilFormat_         = VK_FORMAT_UNDEFINED;
        VKPtr<VkImage>                      depthImage_;
        VKPtr<VkImageView>                  depthImageView_;
        VKDeviceMemoryRegion*               depthImageMemoryRegion_     = nullptr;

        VkQueue                             graphicsQueue_              = VK_NULL_HANDLE;
        VkQueue                             presentQueue_               = VK_NULL_HANDLE;

        VKPtr<VkSemaphore>                  imageAvailableSemaphore_;
        VKPtr<VkSemaphore>                  renderFinishedSemaphore_;

        VKCommandBuffer*                    commandBuffer_              = nullptr;

        VsyncDescriptor                     vsyncDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
