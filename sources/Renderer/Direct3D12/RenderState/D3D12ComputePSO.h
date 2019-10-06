/*
 * D3D12ComputePSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMPUTE_PSO_H
#define LLGL_D3D12_COMPUTE_PSO_H


#include "D3D12PipelineState.h"


namespace LLGL
{


class D3D12Device;
class D3D12ShaderProgram;
class D3D12PipelineLayout;

class D3D12ComputePSO final : public D3D12PipelineState
{

    public:

        D3D12ComputePSO(
            D3D12Device&                        device,
            D3D12PipelineLayout&                defaultPipelineLayout,
            const ComputePipelineDescriptor&    desc
        );

        void Bind(D3D12CommandContext& commandContext) override;

    private:

        void CreateNativePSO(
            D3D12Device&                device,
            const D3D12ShaderProgram&   shaderProgram
        );

};


} // /namespace LLGL


#endif



// ================================================================================
