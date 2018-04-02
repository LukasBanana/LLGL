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

    private:

        VkDevice                device_         = VK_NULL_HANDLE;
        VKPtr<VkDescriptorPool> descriptorPool_;
        VkDescriptorSet         descriptorSet_  = VK_NULL_HANDLE;

};


} // /namespace LLGL


#endif



// ================================================================================
