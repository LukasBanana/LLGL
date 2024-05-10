/*
 * VKGraphicsPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKGraphicsPSO.h"
#include "VKPipelineLayout.h"
#include "VKRenderPass.h"
#include "VKPipelineCache.h"
#include "../Ext/VKExtensionRegistry.h"
#include "../Shader/VKShader.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include <cstddef>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Container/SmallVector.h>
#include "../../../Core/Assertion.h"
#include "../../../Core/StringUtils.h"


namespace LLGL
{


VKGraphicsPSO::VKGraphicsPSO(
    VkDevice                            device,
    const RenderPass*                   defaultRenderPass,
    const GraphicsPipelineDescriptor&   desc,
    const VKGraphicsPipelineLimits&     limits,
    PipelineCache*                      pipelineCache)
:
    VKPipelineState    { device, VK_PIPELINE_BIND_POINT_GRAPHICS, GetShadersAsArray(desc), desc.pipelineLayout },
    scissorEnabled_    { desc.rasterizer.scissorTestEnabled                                                    },
    hasDynamicScissor_ { desc.scissors.empty()                                                                 }
{
    /* Get render pass from descriptor or default render pass */
    const RenderPass* renderPass = (desc.renderPass != nullptr ? desc.renderPass : defaultRenderPass);
    LLGL_ASSERT_PTR(renderPass);

    /* Create Vulkan graphics pipeline object */
    const VKRenderPass* renderPassVK = LLGL_CAST(const VKRenderPass*, renderPass);
    if (VKPipelineCache* pipelineCacheVK = (pipelineCache != nullptr ? LLGL_CAST(VKPipelineCache*, pipelineCache) : nullptr))
        CreateVkPipeline(device, *renderPassVK, limits, desc, pipelineCacheVK->GetNative());
    else
        CreateVkPipeline(device, *renderPassVK, limits, desc);
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
    const std::size_t numViewports = desc.viewports.size();
    const std::size_t numScissors = desc.scissors.size();

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    /* Initialize viewports */
    if (numViewports > 0)
    {
        createInfo.viewportCount = static_cast<std::uint32_t>(numViewports);

        /* Convert viewports to Vulkan structure */
        viewportsVK.resize(numViewports);

        for_range(i, numViewports)
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

        for_range(i, numViewports)
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
        LLGL_ASSERT_VK_EXT(EXT_conservative_rasterization);

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
    createInfo.colorWriteMask       = VKTypes::ToVkColorComponentFlags(desc.colorMask);
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
    for_range(i, numColorAttachments)
    {
        CreateColorBlendAttachmentState(
            attachmentStatesVK[i],
            desc.targets[desc.independentBlendEnabled ? i : 0]
        );
    }

    createInfo.attachmentCount      = numColorAttachments;
    createInfo.pAttachments         = attachmentStatesVK.data();
    createInfo.blendConstants[0]    = desc.blendFactor[0];
    createInfo.blendConstants[1]    = desc.blendFactor[1];
    createInfo.blendConstants[2]    = desc.blendFactor[2];
    createInfo.blendConstants[3]    = desc.blendFactor[3];
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

bool VKGraphicsPSO::CreateVkPipeline(
    VkDevice                            device,
    const VKRenderPass&                 renderPass,
    const VKGraphicsPipelineLimits&     limits,
    const GraphicsPipelineDescriptor&   desc,
    VkPipelineCache                     pipelineCache)
{
    /* Get shader program object */
    const VKShader* vertexShaderVK = LLGL_CAST(const VKShader*, desc.vertexShader);
    if (vertexShaderVK == nullptr)
    {
        GetMutableReport().Errorf("cannot create Vulkan graphics pipeline without vertex shader\n");
        return false;
    }

    auto FillAndAppendShaderStageCreateInfo = [this, &desc](
        Shader*                                             shader,
        SmallVector<VkPipelineShaderStageCreateInfo, 5>&    createInfos,
        bool&                                               outShaderCreationFailed)
    {
        if (shader != nullptr)
        {
            VKShader& shaderVK = LLGL_CAST(VKShader&, *shader);
            const Report* report = shaderVK.GetReport();
            if (report != nullptr && report->HasErrors())
            {
                GetMutableReport().Errorf("Failed to load %s shader into Vulkan graphics pipeline state [%s]\n", ToString(shader->GetType()), GetOptionalDebugName(desc.debugName));
                outShaderCreationFailed = true;
            }
            else
            {
                const std::size_t shaderIndex = createInfos.size();
                createInfos.resize(shaderIndex + 1);
                this->GetShaderCreateInfoAndOptionalPermutation(shaderVK, createInfos.back());
            }
        }
    };

    /* Get shader stages */
    SmallVector<VkPipelineShaderStageCreateInfo, 5> shaderStageCreateInfos;
    bool shaderCreationFailed = false;
    FillAndAppendShaderStageCreateInfo(desc.vertexShader,           shaderStageCreateInfos, shaderCreationFailed);
    FillAndAppendShaderStageCreateInfo(desc.tessControlShader,      shaderStageCreateInfos, shaderCreationFailed);
    FillAndAppendShaderStageCreateInfo(desc.tessEvaluationShader,   shaderStageCreateInfos, shaderCreationFailed);
    FillAndAppendShaderStageCreateInfo(desc.geometryShader,         shaderStageCreateInfos, shaderCreationFailed);
    FillAndAppendShaderStageCreateInfo(desc.fragmentShader,         shaderStageCreateInfos, shaderCreationFailed);
    if (shaderCreationFailed)
        return false;

    /* Initialize vertex input descriptor */
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexShaderVK->FillVertexInputStateCreateInfo(vertexInputCreateInfo);

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
    const VkSampleCountFlagBits sampleCountBits = (desc.rasterizer.multiSampleEnabled ? renderPass.GetSampleCountBits() : VK_SAMPLE_COUNT_1_BIT);
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
        createInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stageCount           = static_cast<std::uint32_t>(shaderStageCreateInfos.size());
        createInfo.pStages              = shaderStageCreateInfos.data();
        createInfo.pVertexInputState    = (&vertexInputCreateInfo);
        createInfo.pInputAssemblyState  = (&inputAssembly);
        createInfo.pTessellationState   = (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ? &tessellationState : nullptr);
        createInfo.pViewportState       = (&viewportState);
        createInfo.pRasterizationState  = (&rasterizerState);
        createInfo.pMultisampleState    = (&multisampleState);
        createInfo.pDepthStencilState   = (&depthStencilState);
        createInfo.pColorBlendState     = (&colorBlendState);
        createInfo.pDynamicState        = (!dynamicStatesVK.empty() ? &dynamicState : nullptr);
        createInfo.layout               = GetVkPipelineLayout();
        createInfo.renderPass           = renderPass.GetVkRenderPass();
        createInfo.subpass              = 0;
        createInfo.basePipelineHandle   = VK_NULL_HANDLE;
        createInfo.basePipelineIndex    = 0;
    }
    VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, nullptr, ReleaseAndGetAddressOfVkPipeline());
    VKThrowIfFailed(result, "failed to create Vulkan graphics pipeline");

    return true;
}


} // /namespace LLGL



// ================================================================================
