/*
 * VKPredicateQueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPredicateQueryHeap.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <algorithm>
#include <cstdint>


namespace LLGL
{


VKPredicateQueryHeap::VKPredicateQueryHeap(
    const VKPtr<VkDevice>&      device,
    VKDeviceMemoryManager&      deviceMemoryManager,
    const QueryHeapDescriptor&  desc)
:
    VKQueryHeap   { device, desc        },
    resultBuffer_ { device              },
    memoryMngr_   { deviceMemoryManager }
{
    /* Create result buffer for occlusion predicates */
    VkBufferCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.size                     = desc.numQueries * sizeof(std::uint32_t);
        createInfo.usage                    = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
    }
    resultBuffer_.CreateVkBufferAndMemoryRegion(
        device,
        createInfo,
        memoryMngr_,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    /* Initialize dirty range with invalidation */
    InvalidateDirtyRange();
}

VKPredicateQueryHeap::~VKPredicateQueryHeap()
{
    resultBuffer_.ReleaseMemoryRegion(memoryMngr_);
}

void VKPredicateQueryHeap::FlushDirtyRange(VkCommandBuffer commandBuffer)
{
    if (HasDirtyRange())
    {
        /* Resolve query data within the dirty range, then invalidate range */
        ResolveData(commandBuffer, dirtyRange_[0], (dirtyRange_[1] - dirtyRange_[0]));
        InvalidateDirtyRange();
    }
}

void VKPredicateQueryHeap::MarkDirtyRange(std::uint32_t firstQuery, std::uint32_t numQueries)
{
    dirtyRange_[0] = std::min(dirtyRange_[0], firstQuery);
    dirtyRange_[1] = std::max(dirtyRange_[1], firstQuery + numQueries);
}

bool VKPredicateQueryHeap::HasDirtyRange() const
{
    return (dirtyRange_[0] < dirtyRange_[1]);
}

bool VKPredicateQueryHeap::InsideDirtyRange(std::uint32_t firstQuery, std::uint32_t numQueries) const
{
    /* Check if the specified queries are inside the dirty range, i.e. if [first, count) overlaps wiht [begin, end) */
    return (firstQuery + numQueries > dirtyRange_[0] && firstQuery < dirtyRange_[1]);
}


/*
 * ======= Private: =======
 */

void VKPredicateQueryHeap::InvalidateDirtyRange()
{
    dirtyRange_[0] = UINT32_MAX;
    dirtyRange_[1] = 0;
}

void VKPredicateQueryHeap::ResolveData(VkCommandBuffer commandBuffer, std::uint32_t firstQuery, std::uint32_t numQueries)
{
    vkCmdCopyQueryPoolResults(
        commandBuffer,
        GetVkQueryPool(),
        firstQuery,
        numQueries,
        GetResultVkBuffer(),
        firstQuery * sizeof(std::uint32_t),
        sizeof(std::uint32_t),
        VK_QUERY_RESULT_WAIT_BIT
    );
}


} // /namespace LLGL



// ================================================================================
