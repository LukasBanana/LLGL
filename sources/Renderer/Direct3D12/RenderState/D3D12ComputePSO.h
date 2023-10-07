/*
 * D3D12ComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMPUTE_PSO_H
#define LLGL_D3D12_COMPUTE_PSO_H


#include "D3D12PipelineState.h"


namespace LLGL
{


class PipelineCache;
class D3D12Device;
class D3D12ShaderProgram;
class D3D12PipelineLayout;
class D3D12PipelineCache;

class D3D12ComputePSO final : public D3D12PipelineState
{

    public:

        D3D12ComputePSO(
            D3D12Device&                        device,
            D3D12PipelineLayout&                defaultPipelineLayout,
            const ComputePipelineDescriptor&    desc,
            PipelineCache*                      pipelineCache           = nullptr
        );

        void Bind(D3D12CommandContext& commandContext) override;

    private:

        void CreateNativePSO(
            D3D12Device&                    device,
            const D3D12_SHADER_BYTECODE&    csBytecode,
            D3D12PipelineCache*             pipelineCache   = nullptr
        );

};


} // /namespace LLGL


#endif



// ================================================================================
