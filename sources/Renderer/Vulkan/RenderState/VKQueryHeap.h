/*
 * VKQueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_QUERY_HEAP_H
#define LLGL_VK_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKQueryHeap final : public QueryHeap
{

    public:

        VKQueryHeap(const VKPtr<VkDevice>& device, const QueryHeapDescriptor& desc);

        // Returns the Vulkan VkQueryPool object.
        inline VkQueryPool GetVkQueryPool() const
        {
            return queryPool_.Get();
        }

        // Returns the number of queries per group.
        inline std::uint32_t GetGroupSize() const
        {
            return groupSize_;
        }

    private:

        VKPtr<VkQueryPool>  queryPool_;
        std::uint32_t       groupSize_  = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
