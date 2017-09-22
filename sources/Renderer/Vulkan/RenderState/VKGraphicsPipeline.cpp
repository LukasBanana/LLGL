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
#include <cstddef>


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

static void CreateInputAssemblyState(const GraphicsPipelineDescriptor& desc, VkPipelineInputAssemblyStateCreateInfo& createInfo)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.topology                 = VKTypes::Map(desc.primitiveTopology);
    createInfo.primitiveRestartEnable   = VK_FALSE;
}

static void CreateTessellationState(const GraphicsPipelineDescriptor& desc, VkPipelineTessellationStateCreateInfo& createInfo)
{
    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.patchControlPoints   = GetPrimitiveTopologyPatchSize(desc.primitiveTopology);
}

static void Convert(VkViewport& dst, const Viewport& src)
{
    dst.x        = src.x;
    dst.y        = src.y;
    dst.width    = src.width;
    dst.height   = src.height;
    dst.minDepth = src.minDepth;
    dst.maxDepth = src.maxDepth;
}

static void Convert(VkRect2D& dst, const Scissor& src)
{
    dst.offset.x        = src.x;
    dst.offset.y        = src.y;
    dst.extent.width    = static_cast<uint32_t>(src.width);
    dst.extent.height   = static_cast<uint32_t>(src.height);
}

static void Convert(VkRect2D& dst, const Viewport& src)
{
    dst.offset.x        = static_cast<int32_t>(src.x);
    dst.offset.y        = static_cast<int32_t>(src.y);
    dst.extent.width    = static_cast<uint32_t>(src.width);
    dst.extent.height   = static_cast<uint32_t>(src.height);
}

static void CreateViewportState(const GraphicsPipelineDescriptor& desc, VkPipelineViewportStateCreateInfo& createInfo, std::vector<VkViewport>& viewportsVK, std::vector<VkRect2D>& scissorsVK)
{
    const auto numViewports = desc.viewports.size();
    const auto numScissors = desc.scissors.size();

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    /* Initialize viewports */
    createInfo.viewportCount = static_cast<uint32_t>(numViewports);

    /* Check if VkViewpport and Viewport structures can be safely reinterpret-casted */
    if ( sizeof(VkViewport)             == sizeof(Viewport)             &&
         offsetof(VkViewport, x)        == offsetof(Viewport, x       ) &&
         offsetof(VkViewport, y)        == offsetof(Viewport, y       ) &&
         offsetof(VkViewport, width   ) == offsetof(Viewport, width   ) &&
         offsetof(VkViewport, height  ) == offsetof(Viewport, height  ) &&
         offsetof(VkViewport, minDepth) == offsetof(Viewport, minDepth) &&
         offsetof(VkViewport, maxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Use viewport descritpors directly */
        createInfo.pViewports = reinterpret_cast<const VkViewport*>(desc.viewports.data());
    }
    else
    {
        /* Convert viewports to Vulkan structure */
        viewportsVK.resize(numViewports);

        for (size_t i = 0; i < numViewports; ++i)
            Convert(viewportsVK[i], desc.viewports[i]);

        createInfo.pViewports = viewportsVK.data();
    }

    /* Convert scissors to Vulkan structure */
    createInfo.scissorCount = static_cast<uint32_t>(numViewports);

    scissorsVK.resize(numViewports);

    for (size_t i = 0; i < numViewports; ++i)
    {
        if (i < numScissors)
            Convert(scissorsVK[i], desc.scissors[i]);
        else
            Convert(scissorsVK[i], desc.viewports[i]);
    }

    createInfo.pScissors = scissorsVK.data();
}

static void CreateRasterizerState(const GraphicsPipelineDescriptor& desc, VkPipelineRasterizationStateCreateInfo& createInfo)
{
    createInfo.sType        = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.pNext        = nullptr;
    createInfo.flags        = 0;


}

static void CreateMultisampleState(const GraphicsPipelineDescriptor& desc, VkPipelineMultisampleStateCreateInfo& createInfo)
{
    createInfo.sType        = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.pNext        = nullptr;
    createInfo.flags        = 0;


}

static void CreateDepthStencilState(const GraphicsPipelineDescriptor& desc, VkPipelineDepthStencilStateCreateInfo& createInfo)
{
    createInfo.sType        = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.pNext        = nullptr;
    createInfo.flags        = 0;


}

static void CreateColorBlendState(const GraphicsPipelineDescriptor& desc, VkPipelineColorBlendStateCreateInfo& createInfo)
{
    createInfo.sType        = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.pNext        = nullptr;
    createInfo.flags        = 0;


}

void VKGraphicsPipeline::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    /* Get shader stages */
    auto shaderStageCreateInfos = shaderProgram_->GetShaderStageCreateInfos();

    /* Initialize vertex input descriptor */
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    shaderProgram_->FillVertexInputStateCreateInfo(vertexInputCreateInfo);

    /* Initialize input assembly state */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    CreateInputAssemblyState(desc, inputAssembly);

    /* Initialize tessellation state */
    VkPipelineTessellationStateCreateInfo tessellationState;
    CreateTessellationState(desc, tessellationState);

    /* Initialize viewport state */
    std::vector<VkViewport> viewportsVK;
    std::vector<VkRect2D> scissorsVK;
    VkPipelineViewportStateCreateInfo viewportState;
    CreateViewportState(desc, viewportState, viewportsVK, scissorsVK);

    /* Initialize rasterizer state */
    VkPipelineRasterizationStateCreateInfo rasterizerState;
    CreateRasterizerState(desc, rasterizerState);

    /* Initialize multi-sample state */
    VkPipelineMultisampleStateCreateInfo multisampleState;
    CreateMultisampleState(desc, multisampleState);

    /* Initialize depth-stencil state */
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    CreateDepthStencilState(desc, depthStencilState);

    /* Initialize color-blend state */
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    CreateColorBlendState(desc, colorBlendState);

    /* Create graphics pipeline state object */
    VkGraphicsPipelineCreateInfo createInfo;

    createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext                        = nullptr;
    createInfo.flags                        = 0;
    createInfo.stageCount                   = static_cast<uint32_t>(shaderStageCreateInfos.size());
    createInfo.pStages                      = shaderStageCreateInfos.data();
    createInfo.pVertexInputState            = (&vertexInputCreateInfo);
    createInfo.pInputAssemblyState          = (&inputAssembly);
    createInfo.pTessellationState           = (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ? &tessellationState : nullptr);
    createInfo.pViewportState               = (&viewportState);
    createInfo.pRasterizationState          = (&rasterizerState);
    createInfo.pMultisampleState            = (&multisampleState);
    createInfo.pDepthStencilState           = (&depthStencilState);
    createInfo.pColorBlendState             = (&colorBlendState);
    createInfo.pDynamicState                = nullptr;
    createInfo.layout                       = pipelineLayout_;
    createInfo.renderPass                   = renderPass_;
    createInfo.subpass                      = 0;
    createInfo.basePipelineHandle           = VK_NULL_HANDLE;
    createInfo.basePipelineIndex            = 0;

    VkResult result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &createInfo, nullptr, pipeline_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan graphics pipeline");
}


} // /namespace LLGL



// ================================================================================
