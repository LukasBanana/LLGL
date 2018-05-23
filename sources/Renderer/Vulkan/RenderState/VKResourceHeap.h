/*
 * VKResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RESOURCE_HEAP_H
#define LLGL_VK_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKBuffer;
struct VKWriteDescriptorContainer;
struct VKLayoutBinding;

class VKResourceHeap : public ResourceHeap
{

    public:

        VKResourceHeap(const VKPtr<VkDevice>& device, const ResourceHeapDescriptor& desc);
        ~VKResourceHeap();

        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_;
        }

        inline VkDescriptorPool GetVkDescriptorPool() const
        {
            return descriptorPool_.Get();
        }

        inline const std::vector<VkDescriptorSet>& GetVkDescriptorSets() const
        {
            return descriptorSets_;
        }

    private:

        void CreateDescriptorPool(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings);
        void CreateDescriptorSets(std::uint32_t numSetLayouts, const VkDescriptorSetLayout* setLayouts);
        void UpdateDescriptorSets(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings);

        void FillWriteDescriptorForSampler(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);
        void FillWriteDescriptorForTexture(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);
        void FillWriteDescriptorForBuffer(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container);

        VkDevice                        device_         = VK_NULL_HANDLE;
        VkPipelineLayout                pipelineLayout_ = VK_NULL_HANDLE;
        VKPtr<VkDescriptorPool>         descriptorPool_;
        std::vector<VkDescriptorSet>    descriptorSets_;

};


} // /namespace LLGL


#endif



// ================================================================================
