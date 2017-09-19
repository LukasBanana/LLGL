/*
 * VKGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKGraphicsPipeline.h"
#include "../Shader/VKShaderProgram.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"


namespace LLGL
{


VKGraphicsPipeline::VKGraphicsPipeline(const VKPtr<VkDevice>& device, VkRenderPass renderPass, const GraphicsPipelineDescriptor& desc) :
    device_         { device                          },
    renderPass_     { renderPass                      },
    pipelineLayout_ { device, vkDestroyPipelineLayout },
    pipeline_       { device, vkDestroyPipeline       }
{
    /* Get shader program object */
    shaderProgram_ = LLGL_CAST(VKShaderProgram*, desc.shaderProgram);

    if (!shaderProgram_)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Create graphics pipeline states */
    CreateGraphicsPipeline(desc);
}


/*
 * ======= Private: =======
 */

void VKGraphicsPipeline::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    /* Get shader stages */
    auto shaderStageCreateInfos = shaderProgram_->GetShaderStageCreateInfos();

    /* Initialize vertex input descriptor */
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    shaderProgram_->FillVertexInputStateCreateInfo(vertexInputCreateInfo);

    /* Initialize input assembly state */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    
    inputAssembly.sType                     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext                     = nullptr;
    inputAssembly.flags                     = 0;
    inputAssembly.topology                  = VKTypes::Map(desc.primitiveTopology);
    inputAssembly.primitiveRestartEnable    = VK_FALSE;

    /* Initialize tessellation state */
    VkPipelineTessellationStateCreateInfo tessellationState;

    tessellationState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.pNext                 = nullptr;
    tessellationState.flags                 = 0;
    tessellationState.patchControlPoints    = GetPrimitiveTopologyPatchSize(desc.primitiveTopology);

    /* Create graphics pipeline state object */
    VkGraphicsPipelineCreateInfo createInfo;

    createInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.stageCount           = static_cast<uint32_t>(shaderStageCreateInfos.size());
    createInfo.pStages              = shaderStageCreateInfos.data();
    createInfo.pVertexInputState    = (&vertexInputCreateInfo);
    createInfo.pInputAssemblyState  = (&inputAssembly);
    createInfo.pTessellationState   = (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ? &tessellationState : nullptr);
    createInfo.pViewportState       = nullptr;
    createInfo.pRasterizationState  = nullptr;
    createInfo.pMultisampleState    = nullptr;
    createInfo.pDepthStencilState   = nullptr;
    createInfo.pColorBlendState     = nullptr;
    createInfo.pDynamicState        = nullptr;
    createInfo.layout               = pipelineLayout_;
    createInfo.renderPass           = renderPass_;
    createInfo.subpass              = 0;
    createInfo.basePipelineHandle   = VK_NULL_HANDLE;
    createInfo.basePipelineIndex    = 0;

    VkResult result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &createInfo, nullptr, pipeline_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan graphics pipeline");
}


} // /namespace LLGL



// ================================================================================
