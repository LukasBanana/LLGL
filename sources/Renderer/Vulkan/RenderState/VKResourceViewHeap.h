/*
 * VKResourceViewHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RESOURCE_VIEW_HEAP_H
#define LLGL_VK_RESOURCE_VIEW_HEAP_H


#include <LLGL/ResourceViewHeap.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKResourceViewHeap : public ResourceViewHeap
{

    public:

        VKResourceViewHeap(const VKPtr<VkDevice>& device, const ResourceViewHeapDescriptor& desc);
        ~VKResourceViewHeap();

        inline VkPipelineLayout GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

        inline VkDescriptorPool GetDescriptorPool() const
        {
            return descriptorPool_.Get();
        }

        inline const std::vector<VkDescriptorSet>& GetDescriptorSets() const
        {
            return descriptorSets_;
        }

    private:

        void CreateDescriptorPool(const ResourceViewHeapDescriptor& desc);
        void CreateDescriptorSets(std::uint32_t numSetLayouts, const VkDescriptorSetLayout* setLayouts);
        void UpdateDescriptorSets(const ResourceViewHeapDescriptor& desc);

        VkDevice                        device_         = VK_NULL_HANDLE;
        VkPipelineLayout                pipelineLayout_ = VK_NULL_HANDLE;
        VKPtr<VkDescriptorPool>         descriptorPool_;
        std::vector<VkDescriptorSet>    descriptorSets_;

};


} // /namespace LLGL


#endif



// ================================================================================
