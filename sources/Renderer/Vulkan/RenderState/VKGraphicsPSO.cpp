/*
 * VKGraphicsPSO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKGraphicsPSO.h"
#include "VKPipelineLayout.h"
#include "VKRenderPass.h"
#include "../Ext/VKExtensionRegistry.h"
#include "../Shader/VKShaderProgram.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include <cstddef>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/StaticLimits.h>


namespace LLGL
{


VKGraphicsPSO::VKGraphicsPSO(
    const VKPtr<VkDevice>&              device,
    VkPipelineLayout                    defaultPipelineLayout,
    const RenderPass*                   defaultRenderPass,
    const GraphicsPipelineDescriptor&   desc,
    const VKGraphicsPipelineLimits&     limits)
:
    VKPipelineState    { device, VK_PIPELINE_BIND_POINT_GRAPHICS },
    scissorEnabled_    { desc.rasterizer.scissorTestEnabled      },
    hasDynamicScissor_ { desc.scissors.empty()                   }
{
    if (auto renderPass = (desc.renderPass != nullptr ? desc.renderPass : defaultRenderPass))
    {
        /* Create Vulkan graphics pipeline object */
        auto renderPassVK = LLGL_CAST(const VKRenderPass*, renderPass);
        CreateVkPipeline(
            device,
            GetVkPipelineLayoutOrDefault(desc.pipelineLayout, defaultPipelineLayout),
            *renderPassVK,
            limits,
            desc
        );
    }
    else
        throw std::invalid_argument("cannot create Vulkan graphics pipeline without render pass");
}


/*
 * ======= Private: =======
 */

static void CreateInputAssemblyState(
    const GraphicsPipelineDescriptor&       desc,
    VkPipelineInputAssemblyStateCreateInfo& createInfo)
{
    /* Always enable primitive restart index for strip topologies, to be compatible with D3D11 and Metal */
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.topology                 = VKTypes::Map(desc.primitiveTopology);
    createInfo.primitiveRestartEnable   = VKBoolean(IsPrimitiveTopologyStrip(desc.primitiveTopology));
}

static void CreateTessellationState(
    const GraphicsPipelineDescriptor&       desc,
    VkPipelineTessellationStateCreateInfo&  createInfo)
{
    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.patchControlPoints   = GetPrimitiveTopologyPatchSize(desc.primitiveTopology);
}

static void CreateViewportState(
    const GraphicsPipelineDescriptor&   desc,
    VkPipelineViewportStateCreateInfo&  createInfo,
    std::vector<VkViewport>&            viewportsVK,
    std::vector<VkRect2D>&              scissorsVK)
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
        /* Set viewport count to 1 (required), but set array to null pointer (will be ignored) */
        createInfo.viewportCount    = 1;
        createInfo.pViewports       = nullptr;
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

        createInfo.pScissors = scissorsVK.data();
    }
    else
    {
        /* Set scissor count to 1 (required), but set array to null pointer (will be ignored) */
        createInfo.scissorCount = 1;
        createInfo.pScissors    = nullptr;
    }
}

static void CreateRasterizerState(
    const RasterizerDescriptor&                             desc,
    const VKGraphicsPipelineLimits&                         limits,
    VkPipelineRasterizationStateCreateInfo&                 createInfo,
    VkPipelineRasterizationConservativeStateCreateInfoEXT&  createInfoConservativeRasterExt)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.depthClampEnable         = VKBoolean(desc.depthClampEnabled);
    createInfo.rasterizerDiscardEnable  = VKBoolean(desc.discardEnabled);
    createInfo.polygonMode              = VKTypes::Map(desc.polygonMode);
    createInfo.cullMode                 = VKTypes::Map(desc.cullMode);
    createInfo.frontFace                = (desc.frontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE);
    createInfo.depthBiasEnable          = VKBoolean(desc.depthBias.constantFactor != 0.0f || desc.depthBias.slopeFactor != 0.0f || desc.depthBias.clamp != 0.0f);
    createInfo.depthBiasConstantFactor  = desc.depthBias.constantFactor;
    createInfo.depthBiasClamp           = desc.depthBias.clamp;
    createInfo.depthBiasSlopeFactor     = desc.depthBias.slopeFactor;
    createInfo.lineWidth                = std::max(limits.lineWidthRange[0], std::min(desc.lineWidth, limits.lineWidthRange[1]));

    if (desc.conservativeRasterization)
    {
        LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_conservative_rasterization, "VK_EXT_conservative_rasterization");

        createInfo.pNext = &createInfoConservativeRasterExt;
        {
            createInfoConservativeRasterExt.sType                               = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
            createInfoConservativeRasterExt.pNext                               = nullptr;
            createInfoConservativeRasterExt.flags                               = 0;
            createInfoConservativeRasterExt.conservativeRasterizationMode       = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
            createInfoConservativeRasterExt.extraPrimitiveOverestimationSize    = 0.0f;
        }
    }
}

static void CreateMultisampleState(
    const VkSampleCountFlagBits             sampleCountBits,
    const BlendDescriptor&                  blendDesc,
    VkPipelineMultisampleStateCreateInfo&   createInfo)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.rasterizationSamples     = sampleCountBits;
    createInfo.sampleShadingEnable      = VK_FALSE;
    createInfo.minSampleShading         = 0.0f;
    createInfo.pSampleMask              = static_cast<const VkSampleMask*>(&(blendDesc.sampleMask));
    createInfo.alphaToCoverageEnable    = VKBoolean(blendDesc.alphaToCoverageEnabled);
    createInfo.alphaToOneEnable         = VK_FALSE;
}

static void CreateStencilOpState(
    const StencilFaceDescriptor&    desc,
    VkStencilOpState&               createInfo)
{
    createInfo.failOp       = VKTypes::Map(desc.stencilFailOp);
    createInfo.passOp       = VKTypes::Map(desc.depthPassOp);
    createInfo.depthFailOp  = VKTypes::Map(desc.depthFailOp);
    createInfo.compareOp    = VKTypes::Map(desc.compareOp);
    createInfo.compareMask  = desc.readMask;
    createInfo.writeMask    = desc.writeMask;
    createInfo.reference    = desc.reference;
}

static void CreateDepthStencilState(
    const GraphicsPipelineDescriptor&       desc,
    VkPipelineDepthStencilStateCreateInfo&  createInfo)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.depthTestEnable          = VKBoolean(desc.depth.testEnabled);
    createInfo.depthWriteEnable         = VKBoolean(desc.depth.writeEnabled);
    createInfo.depthCompareOp           = VKTypes::Map(desc.depth.compareOp);
    createInfo.depthBoundsTestEnable    = VK_FALSE;
    createInfo.stencilTestEnable        = VKBoolean(desc.stencil.testEnabled);
    CreateStencilOpState(desc.stencil.front, createInfo.front);
    CreateStencilOpState(desc.stencil.back, createInfo.back);
    createInfo.minDepthBounds           = 0.0f;
    createInfo.maxDepthBounds           = 1.0f;
}

static void Convert(VkColorComponentFlags& dst, const ColorRGBAb& src)
{
    dst = 0;
    if (src.r) { dst |= VK_COLOR_COMPONENT_R_BIT; }
    if (src.g) { dst |= VK_COLOR_COMPONENT_G_BIT; }
    if (src.b) { dst |= VK_COLOR_COMPONENT_B_BIT; }
    if (src.a) { dst |= VK_COLOR_COMPONENT_A_BIT; }
}

static void CreateColorBlendAttachmentState(
    VkPipelineColorBlendAttachmentState&    createInfo,
    const BlendTargetDescriptor&            desc)
{
    createInfo.blendEnable          = VKBoolean(desc.blendEnabled);
    createInfo.srcColorBlendFactor  = VKTypes::Map(desc.srcColor);
    createInfo.dstColorBlendFactor  = VKTypes::Map(desc.dstColor);
    createInfo.colorBlendOp         = VKTypes::Map(desc.colorArithmetic);
    createInfo.srcAlphaBlendFactor  = VKTypes::Map(desc.srcAlpha);
    createInfo.dstAlphaBlendFactor  = VKTypes::Map(desc.dstAlpha);
    createInfo.alphaBlendOp         = VKTypes::Map(desc.alphaArithmetic);
    Convert(createInfo.colorWriteMask, desc.colorMask);
}

static void CreateColorBlendState(
    const BlendDescriptor&                              desc,
    VkPipelineColorBlendStateCreateInfo&                createInfo,
    std::vector<VkPipelineColorBlendAttachmentState>&   attachmentStatesVK,
    std::uint32_t                                       numColorAttachments)
{
    numColorAttachments = std::min(numColorAttachments, LLGL_MAX_NUM_COLOR_ATTACHMENTS);

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

    /* Convert blend targets to Vulkan structure */
    attachmentStatesVK.resize(numColorAttachments);
    for (std::uint32_t i = 0; i < numColorAttachments; ++i)
    {
        CreateColorBlendAttachmentState(
            attachmentStatesVK[i],
            desc.targets[desc.independentBlendEnabled ? i : 0]
        );
    }

    createInfo.attachmentCount      = numColorAttachments;
    createInfo.pAttachments         = attachmentStatesVK.data();
    createInfo.blendConstants[0]    = desc.blendFactor.r;
    createInfo.blendConstants[1]    = desc.blendFactor.g;
    createInfo.blendConstants[2]    = desc.blendFactor.b;
    createInfo.blendConstants[3]    = desc.blendFactor.a;
}

static void CreateDynamicState(
    const GraphicsPipelineDescriptor&   desc,
    VkPipelineDynamicStateCreateInfo&   createInfo,
    std::vector<VkDynamicState>&        dynamicStatesVK)
{
    if (desc.viewports.empty())
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    if (desc.scissors.empty())
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_SCISSOR);
    if (desc.blend.blendFactorDynamic)
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    if (desc.stencil.referenceDynamic)
        dynamicStatesVK.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.dynamicStateCount    = static_cast<std::uint32_t>(dynamicStatesVK.size());
    createInfo.pDynamicStates       = (dynamicStatesVK.empty() ? nullptr : dynamicStatesVK.data());
}

void VKGraphicsPSO::CreateVkPipeline(
    VkDevice                            device,
    VkPipelineLayout                    pipelineLayout,
    const VKRenderPass&                 renderPass,
    const VKGraphicsPipelineLimits&     limits,
    const GraphicsPipelineDescriptor&   desc)
{
    /* Get shader program object */
    auto shaderProgramVK = LLGL_CAST(const VKShaderProgram*, desc.shaderProgram);
    if (!shaderProgramVK)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Get shader stages */
    std::uint32_t shaderStateCount = 5;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[5];
    shaderProgramVK->FillShaderStageCreateInfos(shaderStageCreateInfos, shaderStateCount);

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
    CreateViewportState(desc, viewportState, viewportsVK, scissorsVK);

    /* Initialize rasterizer state */
    VkPipelineRasterizationStateCreateInfo rasterizerState;
    VkPipelineRasterizationConservativeStateCreateInfoEXT createInfoConservativeRasterExt;
    CreateRasterizerState(desc.rasterizer, limits, rasterizerState, createInfoConservativeRasterExt);

    /* Initialize multi-sample state */
    VkPipelineMultisampleStateCreateInfo multisampleState;
    const auto sampleCountBits = (desc.rasterizer.multiSampleEnabled ? renderPass.GetSampleCountBits() : VK_SAMPLE_COUNT_1_BIT);
    CreateMultisampleState(sampleCountBits, desc.blend, multisampleState);

    /* Initialize depth-stencil state */
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    CreateDepthStencilState(desc, depthStencilState);

    /* Initialize color-blend state */
    std::vector<VkPipelineColorBlendAttachmentState> attachmentStatesVK;
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    CreateColorBlendState(desc.blend, colorBlendState, attachmentStatesVK, renderPass.GetNumColorAttachments());

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
        createInfo.stageCount                   = shaderStateCount;
        createInfo.pStages                      = shaderStageCreateInfos;
        createInfo.pVertexInputState            = (&vertexInputCreateInfo);
        createInfo.pInputAssemblyState          = (&inputAssembly);
        createInfo.pTessellationState           = (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ? &tessellationState : nullptr);
        createInfo.pViewportState               = (&viewportState);
        createInfo.pRasterizationState          = (&rasterizerState);
        createInfo.pMultisampleState            = (&multisampleState);
        createInfo.pDepthStencilState           = (&depthStencilState);
        createInfo.pColorBlendState             = (&colorBlendState);
        createInfo.pDynamicState                = (!dynamicStatesVK.empty() ? &dynamicState : nullptr);
        createInfo.layout                       = pipelineLayout;
        createInfo.renderPass                   = renderPass.GetVkRenderPass();
        createInfo.subpass                      = 0;
        createInfo.basePipelineHandle           = VK_NULL_HANDLE;
        createInfo.basePipelineIndex            = 0;
    }
    auto result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, GetVkPipelineAddress());
    VKThrowIfFailed(result, "failed to create Vulkan graphics pipeline");
}


} // /namespace LLGL



// ================================================================================
