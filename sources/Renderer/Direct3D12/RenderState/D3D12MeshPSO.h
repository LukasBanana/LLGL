/*
 * D3D12MeshPSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_MESH_PIPELINE_H
#define LLGL_D3D12_MESH_PIPELINE_H


#include "D3D12RenderPSOBase.h"


namespace LLGL
{


class D3D12MeshPSO final : public D3D12RenderPSOBase
{

    public:

        // Constructs the graphics PSO with the specified descriptor.
        D3D12MeshPSO(
            ID3D12Device2*                  device,
            D3D12PipelineLayout&            defaultPipelineLayout,
            const MeshPipelineDescriptor&   desc,
            const D3D12RenderPass*          defaultRenderPass,
            PipelineCache*                  pipelineCache           = nullptr
        );

        // Binds this graphics PSO to the specified command context.
        void Bind(D3D12CommandContext& commandContext) override;

    private:

        void CreateNativePSO(
            ID3D12Device2*                  device,
            const D3D12PipelineLayout&      pipelineLayout,
            const D3D12RenderPass*          renderPass,
            const MeshPipelineDescriptor&   desc,
            D3D12PipelineCache*             pipelineCache   = nullptr
        );

        ComPtr<ID3D12PipelineState> CreateNativePSOWithDesc(ID3D12Device2* device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc, const char* debugName);

};


} // /namespace LLGL


#endif



// ================================================================================
