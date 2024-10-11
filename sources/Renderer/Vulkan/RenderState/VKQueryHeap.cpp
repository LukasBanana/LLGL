/*
 * VKQueryHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKQueryHeap.h"
#include "../VKCore.h"
#include "../VKTypes.h"


namespace LLGL
{


// Returns the number of individual queries within a group
static std::uint32_t GetQueryGroupSize(const QueryHeapDescriptor& desc)
{
    return (desc.type == QueryType::TimeElapsed ? 2 : 1);
}

static VkQueryPipelineStatisticFlags GetPipelineStatisticsFlags(const QueryHeapDescriptor& desc)
{
    if (desc.type == QueryType::PipelineStatistics)
    {
        return
        (
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT                     |
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT                   |
            VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT                   |
            VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT                 |
            VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT                  |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT                        |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT                         |
            VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT                 |
            VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT         |
            VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT  |
            VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
        );
    }
    return 0;
}

static VkQueryControlFlags GetQueryControlFlags(const QueryHeapDescriptor& desc)
{
    VkQueryControlFlags flags = 0;

    if (desc.type == QueryType::SamplesPassed)
        flags |= VK_QUERY_CONTROL_PRECISE_BIT;

    return flags;
}

VKQueryHeap::VKQueryHeap(VkDevice device, const QueryHeapDescriptor& desc, bool hasPredicates) :
    QueryHeap      { desc.type                    },
    hasPredicates_ { hasPredicates                },
    queryPool_     { device, vkDestroyQueryPool   },
    controlFlags_  { GetQueryControlFlags(desc)   },
    groupSize_     { GetQueryGroupSize(desc)      },
    numQueries_    { desc.numQueries * groupSize_ }
{
    /* Create query pool object */
    VkQueryPoolCreateInfo createInfo;
    {
        createInfo.sType                = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.queryType            = VKTypes::Map(desc.type);
        createInfo.queryCount           = numQueries_;
        createInfo.pipelineStatistics   = GetPipelineStatisticsFlags(desc);
    }
    auto result = vkCreateQueryPool(device, &createInfo, nullptr, queryPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan query pool");
}


} // /namespace LLGL



// ================================================================================
