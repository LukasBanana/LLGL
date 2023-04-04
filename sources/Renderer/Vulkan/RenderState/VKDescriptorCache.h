/*
 * VKDescriptorCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DESCRIPTOR_CACHE_H
#define LLGL_VK_DESCRIPTOR_CACHE_H


#include "../Vulkan.h"
#include "VKPipelineLayout.h"
#include "VKDescriptorSetWriter.h"
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


class VKBuffer;
class VKTexture;
class VKSampler;
class VKStagingDescriptorSetPool;
struct VKLayoutBinding;

// Vulkan descriptor wrapper to manage dynamic descriptor bindings.
class VKDescriptorCache
{

    public:

        VKDescriptorCache(
            VkDevice                            device,
            VkDescriptorPool                    descriptorPool,
            VkDescriptorSetLayout               setLayout,
            std::uint32_t                       numSizes,
            const VkDescriptorPoolSize*         sizes,
            const ArrayView<VKLayoutBinding>&   bindings
        );

        // Resets the descriptor cache.
        void Reset();

        // Emplaces a descriptor into the cache for the specified resource.
        void EmplaceDescriptor(Resource& resource, const VKLayoutBinding& binding);

        /*
        Flushes all changed descriptor by allocating a new descriptor set.
        Otherwise, no changes took place (i.e. IsInvalidated() is false) and VK_NULL_HANDLE is returned.
        */
        VkDescriptorSet FlushDescriptorSet(VKStagingDescriptorSetPool& pool);

        // Returns true if any cache entries are invalidated and need to be flushed again.
        inline bool IsInvalidated() const
        {
            return dirty_;
        }

    private:

        VkDescriptorBufferInfo* NextBufferInfoOrUpdateCache();
        VkDescriptorImageInfo* NextImageInfoOrUpdateCache();

        void EmplaceBufferDescriptor(VKBuffer& bufferVK, const VKLayoutBinding& binding);
        void EmplaceTextureDescriptor(VKTexture& textureVK, const VKLayoutBinding& binding);
        void EmplaceSamplerDescriptor(VKSampler& samplerVK, const VKLayoutBinding& binding);

        void BuildCopyDescriptors(ArrayView<VKLayoutBinding> bindings);
        void UpdateCopyDescriptorSet(VkDescriptorSet dstSet);

    private:

        VkDevice                                device_         = VK_NULL_HANDLE;
        VkDescriptorSetLayout                   setLayout_      = VK_NULL_HANDLE;
        VkDescriptorSet                         descriptorSet_  = VK_NULL_HANDLE;   // Cached Vulkan descriptor set.
        SmallVector<VkDescriptorPoolSize, 4>    poolSizes_;

        std::uint32_t                           numDescriptors_ = 0;                // Total number of descriptors in cache.
        VKDescriptorSetWriter                   setWriter_;
        SmallVector<VkCopyDescriptorSet, 4>     copyDescs_;

        bool                                    dirty_          = false;

};


} // /namespace LLGL


#endif



// ================================================================================
