/*
 * VKRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_CONTEXT_H
#define LLGL_VK_RENDER_CONTEXT_H


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include "VKCore.h"
#include "VKPtr.h"
#include "RenderState/VKRenderPass.h"
#include "Texture/VKDepthStencilBuffer.h"
#include "Texture/VKColorBuffer.h"
#include <memory>
#include <vector>


namespace LLGL
{


class VKDeviceMemoryManager;
class VKDeviceMemoryRegion;

class VKRenderContext final : public RenderContext
{

    public:

        VKRenderContext(
            const VKPtr<VkInstance>&        instance,
            VkPhysicalDevice                physicalDevice,
            const VKPtr<VkDevice>&          device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            RenderContextDescriptor         desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        // Returns the swap-chain render pass object.
        inline const VKRenderPass& GetSwapChainRenderPass() const
        {
            return swapChainRenderPass_;
        }

        // Returns the secondary Vulkan render pass object.
        inline VkRenderPass GetSecondaryVkRenderPass() const
        {
            return secondaryRenderPass_.GetVkRenderPass();
        }

        // Returns the native VkFramebuffer object that is currently used from swap-chain.
        inline VkFramebuffer GetVkFramebuffer() const
        {
            return swapChainFramebuffers_[presentImageIndex_].Get();
        }

        // Returns the render context resolution as VkExtent2D.
        inline const VkExtent2D& GetVkExtent() const
        {
            return swapChainExtent_;
        }

        // Returns true if this render context has a depth-stencil buffer.
        bool HasDepthStencilBuffer() const;

        // Returns true if this render context has multi-sampling enabled.
        bool HasMultiSampling() const;

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        void CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore);
        void CreatePresentSemaphores();
        void CreateGpuSurface();

        void CreateRenderPass(VKRenderPass& renderPass, bool isSecondary);
        void CreateSecondaryRenderPass();

        void CreateSwapChainRenderPass();
        void CreateSwapChain(const VideoModeDescriptor& videoModeDesc, const VsyncDescriptor& vsyncDesc);
        void CreateSwapChainImageViews();
        void CreateSwapChainFramebuffers();

        void CreateDepthStencilBuffer(const VideoModeDescriptor& videoModeDesc);
        void CreateColorBuffers(const VideoModeDescriptor& videoModeDesc);
        void ReleaseRenderBuffers();

        VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
        VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, const VsyncDescriptor& vsyncDesc) const;
        VkExtent2D PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, std::uint32_t width, std::uint32_t height) const;
        VkFormat PickDepthStencilFormat() const;
        VkFormat PickDepthFormat() const;

        void AcquireNextPresentImage();

    private:

        static const std::uint32_t g_maxNumColorBuffers = 3;

        VkInstance              instance_                                       = VK_NULL_HANDLE;
        VkPhysicalDevice        physicalDevice_                                 = VK_NULL_HANDLE;
        const VKPtr<VkDevice>&  device_;

        VKDeviceMemoryManager&  deviceMemoryMngr_;

        VKPtr<VkSurfaceKHR>     surface_;
        SurfaceSupportDetails   surfaceSupportDetails_;

        VKPtr<VkSwapchainKHR>   swapChain_;
        VKRenderPass            swapChainRenderPass_;
        VkSurfaceFormatKHR      swapChainFormat_                                = {};
        std::uint32_t           swapChainSamples_                               = 1;
        VkExtent2D              swapChainExtent_                                = { 0, 0 };
        VkImage                 swapChainImages_[g_maxNumColorBuffers];
        VKPtr<VkImageView>      swapChainImageViews_[g_maxNumColorBuffers];
        VKPtr<VkFramebuffer>    swapChainFramebuffers_[g_maxNumColorBuffers];

        std::uint32_t           numSwapChainBuffers_                            = 1;
        std::uint32_t           presentImageIndex_                              = 0;

        VKRenderPass            secondaryRenderPass_;
        VKDepthStencilBuffer    depthStencilBuffer_;
        VKColorBuffer           colorBuffers_[g_maxNumColorBuffers];

        VkQueue                 graphicsQueue_                                  = VK_NULL_HANDLE;
        VkQueue                 presentQueue_                                   = VK_NULL_HANDLE;

        VKPtr<VkSemaphore>      imageAvailableSemaphore_;
        VKPtr<VkSemaphore>      renderFinishedSemaphore_;

};


} // /namespace LLGL


#endif



// ================================================================================
