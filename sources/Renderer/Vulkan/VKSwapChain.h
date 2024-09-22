/*
 * VKSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SWAP_CHAIN_H
#define LLGL_VK_SWAP_CHAIN_H


#include <LLGL/Window.h>
#include <LLGL/SwapChain.h>
#include <LLGL/RenderPassFlags.h>
#include "VKCore.h"
#include "VKPtr.h"
#include "RenderState/VKRenderPass.h"
#include "Texture/VKDepthStencilBuffer.h"
#include "Texture/VKColorBuffer.h"
#include <memory>
#include <vector>


namespace LLGL
{


class VKCommandContext;
class VKDeviceMemoryManager;
class VKDeviceMemoryRegion;

class VKSwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        VKSwapChain(
            VkInstance                      instance,
            VkPhysicalDevice                physicalDevice,
            VkDevice                        device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            const RendererInfo&             rendererInfo
        );

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

        // Returns the actual swap buffer index.
        std::uint32_t TranslateSwapIndex(std::uint32_t swapBufferIndex) const;

        // Returns the native VkFramebuffer object that is currently used from swap-chain.
        inline VkFramebuffer GetVkFramebuffer(std::uint32_t swapBufferIndex) const
        {
            return swapChainFramebuffers_[swapBufferIndex].Get();
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

        // Copies the specified backbuffer into the destination image.
        void CopyImage(
            VKCommandContext&       context,
            VkImage                 dstImage,
            VkImageLayout           dstImageLayout,
            const TextureRegion&    dstRegion,
            std::uint32_t           srcColorBuffer,
            const Offset2D&         srcOffset,
            VkFormat                format
        );

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        void CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore);
        void CreateGpuFence(VKPtr<VkFence>& fence);
        void CreatePresentSemaphoresAndFences();
        void CreateGpuSurface();

        void CreateRenderPass(VKRenderPass& renderPass, AttachmentLoadOp loadOp, AttachmentStoreOp storeOp);
        void CreateDefaultAndSecondaryRenderPass();

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

        void AcquireNextColorBuffer();

    private:

        static constexpr std::uint32_t maxNumFramesInFlight = 3;

        VkInstance                          instance_                                   = VK_NULL_HANDLE;
        VkPhysicalDevice                    physicalDevice_                             = VK_NULL_HANDLE;
        VkDevice                            device_;

        VKDeviceMemoryManager&              deviceMemoryMngr_;

        VKPtr<VkSurfaceKHR>                 surface_;
        VKSurfaceSupportDetails             surfaceSupportDetails_;

        VKPtr<VkSwapchainKHR>               swapChain_;
        VKRenderPass                        swapChainRenderPass_;
        VkSurfaceFormatKHR                  swapChainFormat_                            = {};
        std::uint32_t                       swapChainSamples_                           = 1;
        VkExtent2D                          swapChainExtent_                            = { 0, 0 };
        std::vector<VkImage>                swapChainImages_;
        std::vector<VKPtr<VkImageView>>     swapChainImageViews_;
        std::vector<VKPtr<VkFramebuffer>>   swapChainFramebuffers_;

        std::uint32_t                       numPreferredColorBuffers_                   = 2;
        std::uint32_t                       numColorBuffers_                            = 0;
        std::uint32_t                       currentColorBuffer_                         = 0; // determined by vkAcquireNextImageKHR
        std::uint32_t                       currentFrameInFlight_                       = 0; // current index for maximum frames in flight
        std::uint32_t                       vsyncInterval_                              = 0;

        VKRenderPass                        secondaryRenderPass_;
        VkFormat                            depthStencilFormat_                         = VK_FORMAT_UNDEFINED;
        VKDepthStencilBuffer                depthStencilBuffer_;
        std::vector<VKColorBuffer>          colorBuffers_;

        VkQueue                             graphicsQueue_                              = VK_NULL_HANDLE;
        VkQueue                             presentQueue_                               = VK_NULL_HANDLE;

        VKPtr<VkSemaphore>                  imageAvailableSemaphore_[maxNumFramesInFlight];
        VKPtr<VkSemaphore>                  renderFinishedSemaphore_[maxNumFramesInFlight];
        VKPtr<VkFence>                      inFlightFences_[maxNumFramesInFlight];

};


} // /namespace LLGL


#endif



// ================================================================================
