/*
 * VKComputePSO.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COMPUTE_PSO_H
#define LLGL_VK_COMPUTE_PSO_H


#include "VKPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;

class VKComputePSO final : public VKPipelineState
{

    public:

        VKComputePSO(VkDevice device, const ComputePipelineDescriptor& desc);

    private:

        void CreateVkPipeline(VkDevice device, const ComputePipelineDescriptor& desc);

    private:

        VKPtr<VkShaderModule> shaderModulePermutation_;

};


} // /namespace LLGL


#endif



// ================================================================================
