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
#include <memory>
#include <vector>
#include <cstdint>


namespace LLGL
{


class VKQueryHeap;

// Structure to link a query to an alias that is of the same type because Vulkan doesn't support multiple queries of the same type begin active.
struct VKQueryAlias
{
    VKQueryHeap*    queryHeap;
    std::uint32_t   queryIndex;
};

using VKQueryAliasPtr = std::unique_ptr<VKQueryAlias>;

// Base class for Vulkan query heaps (sub class: VKPredicateQueryHeap).
class VKQueryHeap : public QueryHeap
{

    public:

        // Native Vulkan query pool types. This is used to detect duplicate types of queries begin active simultaneously.
        enum PoolType
        {
            PoolType_Occlusion,
            PoolType_Timestamp,
            PoolType_TransformFeedbackStreamExt,
            PoolType_PipelineStatistics,

            PoolType_Num,
        };

    public:

        VKQueryHeap(VkDevice device, const QueryHeapDescriptor& desc, bool hasPredicates = false);

        // Sets a new alias for the specified query index.
        void ResetAlias(std::uint32_t query);
        void SetAlias(std::uint32_t query, const VKQueryAlias& queryAlias);
        const VKQueryAlias* GetAlias(std::uint32_t query) const;

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

        // Returns true if this query heap has any alias entries.
        inline bool IsAliased() const
        {
            return (numAliases_ > 0);
        }

    private:

        VKPtr<VkQueryPool>              queryPool_;
        VkQueryControlFlags             controlFlags_   = 0;
        const bool                      hasPredicates_  = false;
        std::uint32_t                   groupSize_      = 1;
        std::uint32_t                   numQueries_     = 0;
        std::vector<VKQueryAliasPtr>    aliases_;
        std::uint32_t                   numAliases_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
