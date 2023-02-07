/*
 * VKResourceHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RESOURCE_HEAP_H
#define LLGL_VK_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "VKPipelineBarrier.h"
#include "VKPipelineLayout.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class Buffer;
class VKBuffer;
class VKTexture;
struct VKWriteDescriptorContainer;
struct ResourceHeapDescriptor;
struct ResourceViewDescriptor;
struct TextureViewDescriptor;

class VKResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        VKResourceHeap(
            const VKPtr<VkDevice>&                      device,
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );

        std::uint32_t UpdateDescriptors(
            const VKPtr<VkDevice>&                      device,
            std::uint32_t                               firstDescriptor,
            const ArrayView<ResourceViewDescriptor>&    resourceViews
        );

        // Inserts a pipeline barrier command into the command buffer if this resource heap requires it.
        void SubmitPipelineBarrier(VkCommandBuffer commandBuffer, std::uint32_t descriptorSet);

        // Returns the native Vulkan pipeline layout.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_;
        }

        // Returns the native Vulkan descritpor pool.
        inline VkDescriptorPool GetVkDescriptorPool() const
        {
            return descriptorPool_.Get();
        }

        // Returns the list of native Vulkan descriptor sets.
        inline const std::vector<VkDescriptorSet>& GetVkDescriptorSets() const
        {
            return descriptorSets_;
        }

        /*
        Returns the pipeline binding point for this resource heap.
        Note: for Vulkan, currently only one binding point is supported for each resource heap.
        */
        inline VkPipelineBindPoint GetBindPoint() const
        {
            return bindPoint_;
        }

    private:

        static constexpr std::uint32_t invalidViewIndex = 0xFFFF;

        struct VKDescriptorBinding
        {
            std::uint32_t           dstBinding;
            VkDescriptorType        descriptorType;
            VkPipelineStageFlags    stageFlags;
            std::uint32_t           imageViewIndex  : 16; // Index (per descriptor set) to the intermediate VkImageView or 0xFFFF if unused.
            std::uint32_t           bufferViewIndex : 16; // Index (per descriptor set) to the intermediate VkBufferView or 0xFFFF if unused.
        };

    private:

        void CopyLayoutBindings(const ArrayView<VKLayoutBinding>& layoutBindings);
        void CopyLayoutBinding(VKDescriptorBinding& dst, const VKLayoutBinding& src);

        void CreateDescriptorPool(const VKPtr<VkDevice>& device, std::uint32_t numDescriptorSets);

        void CreateDescriptorSets(
            const VKPtr<VkDevice>&  device,
            std::uint32_t           numDescriptorSets,
            VkDescriptorSetLayout   globalSetLayout
        );

        void FillWriteDescriptorWithSampler(
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKDescriptorBinding&      binding,
            VKWriteDescriptorContainer&     container
        );

        void FillWriteDescriptorWithImageView(
            const VKPtr<VkDevice>&          device,
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKDescriptorBinding&      binding,
            VKWriteDescriptorContainer&     container
        );

        void FillWriteDescriptorWithBufferRange(
            const VKPtr<VkDevice>&          device,
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKDescriptorBinding&      binding,
            VKWriteDescriptorContainer&     container
        );

        bool ExchangeBufferBarrier(std::uint32_t descriptorSet, Buffer* resource, const VKDescriptorBinding& binding);
        bool EmplaceBarrier(std::uint32_t descriptorSet, std::uint32_t slot, Resource* resource, VkPipelineStageFlags stageFlags);
        bool RemoveBarrier(std::uint32_t descriptorSet, std::uint32_t slot);

        // Returns the image view for the specified texture or creates one if the texture-view is enabled.
        VkImageView GetOrCreateImageView(
            const VKPtr<VkDevice>&          device,
            VKTexture&                      textureVK,
            const ResourceViewDescriptor&   desc,
            std::size_t                     imageViewIndex
        );

    private:

        VkPipelineLayout                    pipelineLayout_         = VK_NULL_HANDLE;

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::vector<VkDescriptorSet>        descriptorSets_;
        SmallVector<VKDescriptorBinding>    bindings_;

        std::vector<VKPtr<VkImageView>>     imageViews_;
      //std::vector<VKPtr<VkBufferView>>    bufferViews_;
        std::uint32_t                       numImageViewsPerSet_    = 0;
        std::uint32_t                       numBufferViewsPerSet_   = 0;

        std::vector<VKPipelineBarrierPtr>   barriers_;
        VkPipelineBindPoint                 bindPoint_              = VK_PIPELINE_BIND_POINT_MAX_ENUM;

};


} // /namespace LLGL


#endif



// ================================================================================
