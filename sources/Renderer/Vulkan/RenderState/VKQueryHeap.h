/*
 * VKQueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_QUERY_HEAP_H
#define LLGL_VK_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


// Base class for Vulkan query heaps (sub class: VKPredicateQueryHeap).
class VKQueryHeap : public QueryHeap
{

    public:

        VKQueryHeap(VkDevice device, const QueryHeapDescriptor& desc, bool hasPredicates = false);

        // Returns the Vulkan VkQueryPool object.
        inline VkQueryPool GetVkQueryPool() const
        {
            return queryPool_.Get();
        }

        // Returns the control flags to be used for vkCmdBeginQuery.
        inline VkQueryControlFlags GetControlFlags() const
        {
            return controlFlags_;
        }

        // Returns the number of queries per group.
        inline std::uint32_t GetGroupSize() const
        {
            return groupSize_;
        }

        // Returns the number of native queries.
        inline std::uint32_t GetNumQueries() const
        {
            return numQueries_;
        }

        // Returns true if this query heap has predicates for conditional rendering, i.e. it can be casted to <VKPredicateQueryHeap>.
        inline bool HasPredicates() const
        {
            return hasPredicates_;
        }

    private:

        VKPtr<VkQueryPool>  queryPool_;
        VkQueryControlFlags controlFlags_   = 0;
        const bool          hasPredicates_  = false;
        std::uint32_t       groupSize_      = 1;
        std::uint32_t       numQueries_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
