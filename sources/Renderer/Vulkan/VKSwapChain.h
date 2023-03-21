/*
 * VKSwapChain.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SWAP_CHAIN_H
#define LLGL_VK_SWAP_CHAIN_H


#include <LLGL/Window.h>
#include <LLGL/SwapChain.h>
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

class VKSwapChain final : public SwapChain
{

    public:

        VKSwapChain(
            VkInstance                      instance,
            VkPhysicalDevice                physicalDevice,
            VkDevice                        device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

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
            return swapChainFramebuffers_[currentColorBuffer_].Get();
        }

        // Returns the swap-chain resolution as VkExtent2D.
        inline const VkExtent2D& GetVkExtent() const
        {
            return swapChainExtent_;
        }

        // Returns true if this swap-chain has a depth-stencil buffer.
        bool HasDepthStencilBuffer() const;

        // Returns true if this swap-chain has multi-sampling enabled.
        bool HasMultiSampling() const;

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        void CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore);
        void CreatePresentSemaphores();
        void CreateGpuSurface();

        void CreateRenderPass(VKRenderPass& renderPass, bool isSecondary);
        void CreateSecondaryRenderPass();

        void CreateSwapChainRenderPass();
        void CreateSwapChain(const Extent2D& resolution, std::uint32_t vsyncInterval);
        void CreateSwapChainImageViews();
        void CreateSwapChainFramebuffers();

        void CreateDepthStencilBuffer(const Extent2D& resolution);
        void CreateColorBuffers(const Extent2D& resolution);
        void ReleaseRenderBuffers();

        void CreateResolutionDependentResources(const Extent2D& resolution);

        VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
        VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, std::uint32_t vsyncInterval) const;
        VkExtent2D PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, const Extent2D& resolution) const;
        VkFormat PickDepthStencilFormat(int depthBits, int stencilBits) const;
        std::uint32_t PickSwapChainSize(std::uint32_t swapBuffers) const;

    private:

        static const std::uint32_t maxNumColorBuffers = 3;

        VkInstance              instance_                                   = VK_NULL_HANDLE;
        VkPhysicalDevice        physicalDevice_                             = VK_NULL_HANDLE;
        VkDevice                device_;

        VKDeviceMemoryManager&  deviceMemoryMngr_;

        VKPtr<VkSurfaceKHR>     surface_;
        SurfaceSupportDetails   surfaceSupportDetails_;

        VKPtr<VkSwapchainKHR>   swapChain_;
        VKRenderPass            swapChainRenderPass_;
        VkSurfaceFormatKHR      swapChainFormat_                            = {};
        std::uint32_t           swapChainSamples_                           = 1;
        VkExtent2D              swapChainExtent_                            = { 0, 0 };
        VkImage                 swapChainImages_[maxNumColorBuffers];
        VKPtr<VkImageView>      swapChainImageViews_[maxNumColorBuffers];
        VKPtr<VkFramebuffer>    swapChainFramebuffers_[maxNumColorBuffers];

        std::uint32_t           numColorBuffers_                            = 2;
        std::uint32_t           currentColorBuffer_                         = 0;
        std::uint32_t           vsyncInterval_                              = 0;

        VKRenderPass            secondaryRenderPass_;
        VkFormat                depthStencilFormat_                         = VK_FORMAT_UNDEFINED;
        VKDepthStencilBuffer    depthStencilBuffer_;
        VKColorBuffer           colorBuffers_[maxNumColorBuffers];

        VkQueue                 graphicsQueue_                              = VK_NULL_HANDLE;
        VkQueue                 presentQueue_                               = VK_NULL_HANDLE;

        VKPtr<VkSemaphore>      imageAvailableSemaphore_[maxNumColorBuffers];
        VKPtr<VkSemaphore>      renderFinishedSemaphore_[maxNumColorBuffers];

};


} // /namespace LLGL


#endif



// ================================================================================
