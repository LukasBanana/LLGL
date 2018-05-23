/*
 * VKGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKGraphicsPipeline.h"
#include "VKPipelineLayout.h"
#include "../Shader/VKShaderProgram.h"
#include "../Texture/VKRenderTarget.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include <cstddef>
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


VKGraphicsPipeline::VKGraphicsPipeline(
    const VKPtr<VkDevice>& device, VkRenderPass renderPass, VkPipelineLayout defaultPipelineLayout,
    const GraphicsPipelineDescriptor& desc, const VKGraphicsPipelineLimits& limits, const VkExtent2D& extent) :
        device_            { device                             },
        renderPass_        { renderPass                         },
        pipelineLayout_    { defaultPipelineLayout              },
        pipeline_          { device, vkDestroyPipeline          },
        scissorEnabled_    { desc.rasterizer.scissorTestEnabled },
        hasDynamicScissor_ { desc.scissors.empty()              }
{
    /* Get pipeline layout object */
    if (desc.pipelineLayout)
    {
        auto pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
        pipelineLayout_ = pipelineLayoutVK->GetVkPipelineLayout();
    }

    /* Use render pass from render target if it's specified */
    if (desc.renderTarget)
    {
        auto renderTargetVK = LLGL_CAST(VKRenderTarget*, desc.renderTarget);
        renderPass_ = renderTargetVK->GetVkRenderPass();
    }

    /* Create Vulkan graphics pipeline object */
    CreateGraphicsPipeline(desc, limits, extent);
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

static void CreateViewportState(
    const GraphicsPipelineDescriptor& desc, const VkExtent2D& extent,
    VkPipelineViewportStateCreateInfo& createInfo, std::vector<VkViewport>& viewportsVK, std::vector<VkRect2D>& scissorsVK)
{
    const auto numViewports = desc.viewports.size();
    const auto numScissors = desc.scissors.size();

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    /* Initialize viewports */
    if (numViewports > 0)
    {
        createInfo.viewportCount = static_cast<std::uint32_t>(numViewports);

        /* Convert viewports to Vulkan structure */
        viewportsVK.resize(numViewports);

        for (size_t i = 0; i < numViewports; ++i)
            VKTypes::Convert(viewportsVK[i], desc.viewports[i]);

        createInfo.pViewports = viewportsVK.data();
    }
    else
    {
        /* Create default viewport */
        createInfo.viewportCount = 1;
        viewportsVK.resize(1);

        VKTypes::Convert(viewportsVK[0], Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));

        createInfo.pViewports = viewportsVK.data();
    }

    /* Convert scissors to Vulkan structure */
    if (numViewports > 0)
    {
        createInfo.scissorCount = static_cast<std::uint32_t>(numViewports);
        scissorsVK.resize(numViewports);

        for (size_t i = 0; i < numViewports; ++i)
        {
            if (i < numScissors)
                VKTypes::Convert(scissorsVK[i], desc.scissors[i]);
            else
                VKTypes::Convert(scissorsVK[i], desc.viewports[i]);
        }
    }
    else
    {
        /* Create default scissor */
        createInfo.scissorCount = 1;
        scissorsVK.resize(1);
        VKTypes::Convert(scissorsVK[0], Scissor(0, 0, extent.width, extent.height));
    }

    createInfo.pScissors = scissorsVK.data();
}

static void CreateRasterizerState(const GraphicsPipelineDescriptor& desc, const VKGraphicsPipelineLimits& limits, VkPipelineRasterizationStateCreateInfo& createInfo)
{
    auto shaderProgramVK = LLGL_CAST(VKShaderProgram*, desc.shaderProgram);

    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.depthClampEnable         = VKBoolean(desc.rasterizer.depthClampEnabled);
    createInfo.rasterizerDiscardEnable  = VKBoolean(!shaderProgramVK->HasFragmentShader());
    createInfo.polygonMode              = VKTypes::Map(desc.rasterizer.polygonMode);
    createInfo.cullMode                 = VKTypes::Map(desc.rasterizer.cullMode);
    #if 1//TODO: make this optional for the user (for now: flip front face to overcome flipped viewport)
    createInfo.frontFace                = (desc.rasterizer.frontCCW ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE);
    #else
    createInfo.frontFace                = (desc.rasterizer.frontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE);
    #endif
    createInfo.depthBiasEnable          = VKBoolean(desc.rasterizer.depthBias.clamp != 0.0f);
    createInfo.depthBiasConstantFactor  = desc.rasterizer.depthBias.constantFactor;
    createInfo.depthBiasClamp           = desc.rasterizer.depthBias.clamp;
    createInfo.depthBiasSlopeFactor     = desc.rasterizer.depthBias.slopeFactor;
    createInfo.lineWidth                = std::max(limits.lineWidthRange[0], std::min(desc.rasterizer.lineWidth, limits.lineWidthRange[1]));
}

static VkSampleCountFlagBits GetSampleCountBitmask(const MultiSamplingDescriptor& desc)
{
    #if 0//TODO: multisampling currently not supported for Vulkan
    if (desc.enabled)
    {
        switch (desc.samples)
        {
            case 1:  return VK_SAMPLE_COUNT_1_BIT;
            case 2:  return VK_SAMPLE_COUNT_2_BIT;
            case 4:  return VK_SAMPLE_COUNT_4_BIT;
            case 8:  return VK_SAMPLE_COUNT_8_BIT;
            case 16: return VK_SAMPLE_COUNT_16_BIT;
            case 32: return VK_SAMPLE_COUNT_32_BIT;
            case 64: return VK_SAMPLE_COUNT_64_BIT;
        }
    }
    #endif
    return VK_SAMPLE_COUNT_1_BIT;
}

static void CreateMultisampleState(const MultiSamplingDescriptor& desc, VkPipelineMultisampleStateCreateInfo& createInfo)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.rasterizationSamples     = GetSampleCountBitmask(desc);
    createInfo.sampleShadingEnable      = VK_FALSE;
    createInfo.minSampleShading         = 0.0f;
    createInfo.pSampleMask              = nullptr;
    createInfo.alphaToCoverageEnable    = VK_FALSE;
    createInfo.alphaToOneEnable         = VK_FALSE;
}

static void CreateStencilOpState(const StencilFaceDescriptor& desc, VkStencilOpState& createInfo)
{
    createInfo.failOp       = VKTypes::Map(desc.stencilFailOp);
    createInfo.passOp       = VKTypes::Map(desc.depthPassOp);
    createInfo.depthFailOp  = VKTypes::Map(desc.depthFailOp);
    createInfo.compareOp    = VKTypes::Map(desc.compareOp);
    createInfo.compareMask  = desc.readMask;
    createInfo.writeMask    = desc.writeMask;
    createInfo.reference    = desc.reference;
}

static void CreateDepthStencilState(const GraphicsPipelineDescriptor& desc, VkPipelineDepthStencilStateCreateInfo& createInfo)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.depthTestEnable          = VKBoolean(desc.depth.testEnabled);
    createInfo.depthWriteEnable         = VKBoolean(desc.depth.writeEnabled);
    createInfo.depthCompareOp           = VKTypes::Map(desc.depth.compareOp);
    createInfo.depthBoundsTestEnable    = VKBoolean(!desc.rasterizer.depthClampEnabled);
    createInfo.stencilTestEnable        = VKBoolean(desc.stencil.testEnabled);
    CreateStencilOpState(desc.stencil.front, createInfo.front);
    CreateStencilOpState(desc.stencil.back, createInfo.back);
    createInfo.minDepthBounds           = 0.0f;
    createInfo.maxDepthBounds           = 1.0f;
}

static void CreateColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& createInfo, const BlendTargetDescriptor& desc, VkBool32 blendEnable)
{
    createInfo.blendEnable          = blendEnable;
    createInfo.srcColorBlendFactor  = VKTypes::Map(desc.srcColor);
    createInfo.dstColorBlendFactor  = VKTypes::Map(desc.destColor);
    createInfo.colorBlendOp         = VKTypes::Map(desc.colorArithmetic);
    createInfo.srcAlphaBlendFactor  = VKTypes::Map(desc.srcAlpha);
    createInfo.dstAlphaBlendFactor  = VKTypes::Map(desc.destAlpha);
    createInfo.alphaBlendOp         = VKTypes::Map(desc.alphaArithmetic);
    createInfo.colorWriteMask       = 0;

    if (desc.colorMask.r)
        createInfo.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
    if (desc.colorMask.g)
        createInfo.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
    if (desc.colorMask.b)
        createInfo.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
    if (desc.colorMask.a)
        createInfo.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
}

static void CreateColorBlendState(const BlendDescriptor& desc, VkPipelineColorBlendStateCreateInfo& createInfo, std::vector<VkPipelineColorBlendAttachmentState>& attachmentStatesVK)
{
    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;

    if (desc.logicOp != LogicOp::Disabled)
    {
        createInfo.logicOpEnable    = VK_TRUE;
        createInfo.logicOp          = VKTypes::Map(desc.logicOp);
    }
    else
    {
        createInfo.logicOpEnable    = VK_FALSE;
        createInfo.logicOp          = VK_LOGIC_OP_NO_OP;
    }

    auto numAttachments = desc.targets.size();

    if (numAttachments > 0)
    {
        /* Convert blend targets to Vulkan structure */
        attachmentStatesVK.resize(numAttachments);
        for (size_t i = 0; i < numAttachments; ++i)
            CreateColorBlendAttachmentState(attachmentStatesVK[i], desc.targets[i], VKBoolean(desc.blendEnabled));
    }
    else
    {
        /* Use default values for a single attachment */
        numAttachments = 1;
        attachmentStatesVK.resize(1);
        CreateColorBlendAttachmentState(attachmentStatesVK[0], {}, VKBoolean(desc.blendEnabled));
    }

    createInfo.attachmentCount      = static_cast<std::uint32_t>(numAttachments);
    createInfo.pAttachments         = attachmentStatesVK.data();

    createInfo.blendConstants[0]    = desc.blendFactor.r;
    createInfo.blendConstants[1]    = desc.blendFactor.g;
    createInfo.blendConstants[2]    = desc.blendFactor.b;
    createInfo.blendConstants[3]    = desc.blendFactor.a;
}

static void CreateDynamicState(const GraphicsPipelineDescriptor& desc, VkPipelineDynamicStateCreateInfo& createInfo, std::vector<VkDynamicState>& dynamicStatesVK)
{
    if (desc.viewports.empty())
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    if (desc.scissors.empty())
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_SCISSOR);

    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.dynamicStateCount    = static_cast<std::uint32_t>(dynamicStatesVK.size());
    createInfo.pDynamicStates       = (dynamicStatesVK.empty() ? nullptr : dynamicStatesVK.data());
}

void VKGraphicsPipeline::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const VKGraphicsPipelineLimits& limits, const VkExtent2D& extent)
{
    /* Get shader program object */
    auto shaderProgramVK = LLGL_CAST(VKShaderProgram*, desc.shaderProgram);
    if (!shaderProgramVK)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Get shader stages */
    auto shaderStageCreateInfos = shaderProgramVK->GetShaderStageCreateInfos();

    /* Initialize vertex input descriptor */
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    shaderProgramVK->FillVertexInputStateCreateInfo(vertexInputCreateInfo);

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
    CreateViewportState(desc, extent, viewportState, viewportsVK, scissorsVK);

    /* Initialize rasterizer state */
    VkPipelineRasterizationStateCreateInfo rasterizerState;
    CreateRasterizerState(desc, limits, rasterizerState);

    /* Initialize multi-sample state */
    VkPipelineMultisampleStateCreateInfo multisampleState;
    CreateMultisampleState(desc.rasterizer.multiSampling, multisampleState);

    /* Initialize depth-stencil state */
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    CreateDepthStencilState(desc, depthStencilState);

    /* Initialize color-blend state */
    std::vector<VkPipelineColorBlendAttachmentState> attachmentStatesVK;
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    CreateColorBlendState(desc.blend, colorBlendState, attachmentStatesVK);

    /* Initialize dynamic state */
    std::vector<VkDynamicState> dynamicStatesVK;
    VkPipelineDynamicStateCreateInfo dynamicState;
    CreateDynamicState(desc, dynamicState, dynamicStatesVK);

    /* Create graphics pipeline state object */
    VkGraphicsPipelineCreateInfo createInfo;
    {
        createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext                        = nullptr;
        createInfo.flags                        = 0;
        createInfo.stageCount                   = static_cast<std::uint32_t>(shaderStageCreateInfos.size());
        createInfo.pStages                      = shaderStageCreateInfos.data();
        createInfo.pVertexInputState            = (&vertexInputCreateInfo);
        createInfo.pInputAssemblyState          = (&inputAssembly);
        createInfo.pTessellationState           = (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ? &tessellationState : nullptr);
        createInfo.pViewportState               = (&viewportState);
        createInfo.pRasterizationState          = (&rasterizerState);
        createInfo.pMultisampleState            = (&multisampleState);
        createInfo.pDepthStencilState           = (&depthStencilState);
        createInfo.pColorBlendState             = (&colorBlendState);
        createInfo.pDynamicState                = (!dynamicStatesVK.empty() ? &dynamicState : nullptr);
        createInfo.layout                       = pipelineLayout_;
        createInfo.renderPass                   = renderPass_;
        createInfo.subpass                      = 0;
        createInfo.basePipelineHandle           = VK_NULL_HANDLE;
        createInfo.basePipelineIndex            = 0;
    }
    auto result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &createInfo, nullptr, pipeline_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan graphics pipeline");
}


} // /namespace LLGL



// ================================================================================
