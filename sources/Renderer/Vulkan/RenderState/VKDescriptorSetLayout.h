/*
 * VKDescriptorSetLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DESCRIPOTOR_SET_LAYOUT_H
#define LLGL_VK_DESCRIPOTOR_SET_LAYOUT_H


#include "../Vulkan.h"
#include "../VKPtr.h"
#include <LLGL/Container/ArrayView.h>
#include <vector>
#include <cstdint>


namespace LLGL
{

    
struct VKLayoutBinding
{
    std::uint32_t           dstBinding;
    std::uint32_t           dstArrayElement;
    VkDescriptorType        descriptorType;
    VkPipelineStageFlags    stageFlags;
    long                    bindFlags;
};

// Wrapper to manager native Vulkan descriptor set layouts.
class VKDescriptorSetLayout
{

    public:

        VKDescriptorSetLayout(VkDevice device);

        VKDescriptorSetLayout(VKDescriptorSetLayout&& rhs) noexcept;

        VKDescriptorSetLayout(const VKDescriptorSetLayout&) = delete;
        VKDescriptorSetLayout& operator = (const VKDescriptorSetLayout&) = delete;

    public:

        void Initialize(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>&& setLayoutBindings);

        void UpdateLayoutBindingType(std::uint32_t descriptorIndex, VkDescriptorType descriptorType);
        void FinalizeUpdateLayoutBindingTypes(VkDevice device);

        void GetLayoutBindings(std::vector<VKLayoutBinding>& outBindings) const;

        // Returns the native VkPipelineLayout object.
        inline VkDescriptorSetLayout GetVkDescriptorSetLayout() const
        {
            return setLayout_.Get();
        }

        inline const std::vector<VkDescriptorSetLayoutBinding>& GetVkLayoutBindings() const
        {
            return setLayoutBindings_;
        }

    public:

        static void CreateVkDescriptorSetLayout(
            VkDevice                                        device,
            const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings,
            VKPtr<VkDescriptorSetLayout>&                   outDescriptorSetLayout
        );

        static int CompareSWO(const VKDescriptorSetLayout& lhs, const VKDescriptorSetLayout& rhs);
        static int CompareSWO(const VKDescriptorSetLayout& lhs, const std::vector<VkDescriptorSetLayoutBinding>& rhs);

    private:

        // Modifies binding slots that overlap with others since Vulkan needs to have unique binding slots within the same descriptor set.
        void SanitizeBindingSlots();

        void CreateVkDescriptorSetLayout(VkDevice device);

    private:

        VKPtr<VkDescriptorSetLayout>                setLayout_;
        std::vector<VkDescriptorSetLayoutBinding>   setLayoutBindings_;
        bool                                        isAnyDescriptorTypeDirty_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
