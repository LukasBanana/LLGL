/*
 * VKPredicateQueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PREDICATE_QUERY_HEAP_H
#define LLGL_VK_PREDICATE_QUERY_HEAP_H


#include "VKQueryHeap.h"
#include "../Buffer/VKDeviceBuffer.h"
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKDeviceMemoryManager;

class VKPredicateQueryHeap final : public VKQueryHeap
{

    public:

        VKPredicateQueryHeap(
            const VKPtr<VkDevice>&      device,
            VKDeviceMemoryManager&      deviceMemoryManager,
            const QueryHeapDescriptor&  desc
        );
        ~VKPredicateQueryHeap();

        // Copies all query predicates into the result buffer if not already done.
        void FlushDirtyRange(VkCommandBuffer commandBuffer);

        // Makrs the specified range of queries as 'dirty', i.e. they need to be resolved before their predicate can be used for conditional rendering.
        void MarkDirtyRange(std::uint32_t firstQuery, std::uint32_t numQueries);

        // Returns true if this query heap has a dirty range that must be resolved before the query data can be retrieved.
        bool HasDirtyRange() const;

        // Returns true if the specified range of queries overlaps with the dirty range.
        bool InsideDirtyRange(std::uint32_t firstQuery, std::uint32_t numQueries) const;

        // Returns the native VkBuffer handle of the result buffer (only for conditional rendering).
        inline VkBuffer GetResultVkBuffer() const
        {
            return resultBuffer_.GetVkBuffer();
        }

    private:

        void InvalidateDirtyRange();
        void ResolveData(VkCommandBuffer commandBuffer, std::uint32_t firstQuery, std::uint32_t numQueries);

    private:

        VKDeviceBuffer          resultBuffer_;
        VKDeviceMemoryManager&  memoryMngr_;
        std::uint32_t           dirtyRange_[2]  = {};   // Begin/end range of queries that need to be copied into the result buffer

};


} // /namespace LLGL


#endif



// ================================================================================
