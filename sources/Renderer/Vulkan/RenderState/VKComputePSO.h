/*
 * VKComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMPUTE_PSO_H
#define LLGL_VK_COMPUTE_PSO_H


#include "VKPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;
class PipelineCache;

class VKComputePSO final : public VKPipelineState
{

    public:

        VKComputePSO(
            VkDevice                            device,
            const ComputePipelineDescriptor&    desc,
            PipelineCache*                      pipelineCache = nullptr
        );

    private:

        bool CreateVkPipeline(
            VkDevice                            device,
            const ComputePipelineDescriptor&    desc,
            VkPipelineCache                     pipelineCache = VK_NULL_HANDLE
        );

};


} // /namespace LLGL


#endif



// ================================================================================
