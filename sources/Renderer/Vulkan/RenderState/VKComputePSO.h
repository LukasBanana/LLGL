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

        VKComputePSO(
            const VKPtr<VkDevice>&              device,
            const ComputePipelineDescriptor&    desc,
            VkPipelineLayout                    defaultPipelineLayout
        );

    private:

        void CreateVkPipeline(
            VkDevice                            device,
            VkPipelineLayout                    pipelineLayout,
            const ComputePipelineDescriptor&    desc
        );

};


} // /namespace LLGL


#endif



// ================================================================================
