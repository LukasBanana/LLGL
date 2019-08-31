/*
 * D3D12ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMPUTE_PIPELINE_H
#define LLGL_D3D12_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>
#include <LLGL/ForwardDecls.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <memory>


namespace LLGL
{


class D3D12Device;
class D3D12ShaderProgram;
class D3D12CommandContext;

class D3D12ComputePipeline final : public ComputePipeline
{

    public:

        void SetName(const char* name) override;

    public:

        D3D12ComputePipeline(
            D3D12Device&                        device,
            ID3D12RootSignature*                defaultRootSignature,
            const ComputePipelineDescriptor&    desc
        );

        void Bind(D3D12CommandContext& commandContext);

    private:

        void CreatePipelineState(
            D3D12Device&                device,
            const D3D12ShaderProgram&   shaderProgram,
            ID3D12RootSignature*        rootSignature
        );

    private:

        ComPtr<ID3D12PipelineState> pipelineState_;
        ID3D12RootSignature*        rootSignature_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
