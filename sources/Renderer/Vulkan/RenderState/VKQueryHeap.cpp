/*
 * VKQueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKQueryHeap.h"
#include "../VKCore.h"
#include "../VKTypes.h"


namespace LLGL
{


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

VKQueryHeap::VKQueryHeap(const VKPtr<VkDevice>& device, const QueryHeapDescriptor& desc) :
    QueryHeap      { desc.type                  },
    queryPool_ { device, vkDestroyQueryPool }
{
    /* Create query pool object */
    VkQueryPoolCreateInfo createInfo;
    {
        createInfo.sType                = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.queryType            = VKTypes::Map(desc.type);
        createInfo.queryCount           = desc.numQueries;
        createInfo.pipelineStatistics   = GetPipelineStatisticsFlags(desc);
    }
    auto result = vkCreateQueryPool(device, &createInfo, nullptr, queryPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan query pool");
}


} // /namespace LLGL



// ================================================================================
