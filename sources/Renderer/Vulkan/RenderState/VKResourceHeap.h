/*
 * VKResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RESOURCE_HEAP_H
#define LLGL_VK_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include "VKPipelineBarrier.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKBuffer;
struct VKWriteDescriptorContainer;
struct VKLayoutBinding;
struct ResourceHeapDescriptor;
struct ResourceViewDescriptor;

class VKResourceHeap final : public ResourceHeap
{

    public:

        VKResourceHeap(const VKPtr<VkDevice>& device, const ResourceHeapDescriptor& desc);
        ~VKResourceHeap();

        // Inserts a pipeline barrier command into the command buffer if this resource heap requires it.
        void InsertPipelineBarrier(VkCommandBuffer commandBuffer);

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

        void CreateDescriptorPool(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings);
        void CreateDescriptorSets(std::uint32_t numSetLayouts, const VkDescriptorSetLayout* setLayouts);
        void UpdateDescriptorSets(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings);

        void FillWriteDescriptorForSampler(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);
        void FillWriteDescriptorForTexture(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);
        void FillWriteDescriptorForBuffer(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);

        void CreatePipelineBarrier(const std::vector<ResourceViewDescriptor>& resourceViews, const std::vector<VKLayoutBinding>& bindings);

    private:

        VkDevice                        device_         = VK_NULL_HANDLE;
        VkPipelineLayout                pipelineLayout_ = VK_NULL_HANDLE;
        VKPtr<VkDescriptorPool>         descriptorPool_;
        std::vector<VkDescriptorSet>    descriptorSets_;
        VKPipelineBarrier               barrier_;
        VkPipelineBindPoint             bindPoint_      = VK_PIPELINE_BIND_POINT_MAX_ENUM;


};


} // /namespace LLGL


#endif



// ================================================================================
