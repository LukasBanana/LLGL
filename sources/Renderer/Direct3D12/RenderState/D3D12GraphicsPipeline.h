/*
 * D3D12GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_GRAPHICS_PIPELINE_H
#define LLGL_D3D12_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


struct GraphicsPipelineDescriptor;
class D3D12Device;
class D3D12ShaderProgram;

class D3D12GraphicsPipeline final : public GraphicsPipeline
{

    public:

        D3D12GraphicsPipeline(
            D3D12Device&                        device,
            ID3D12RootSignature*                defaultRootSignature,
            const GraphicsPipelineDescriptor&   desc
        );

        void Bind(ID3D12GraphicsCommandList* commandList);

        // Returns true if scissors are enabled.
        inline bool IsScissorEnabled() const
        {
            return scissorEnabled_;
        }

    private:

        void CreatePipelineState(
            D3D12Device&                        device,
            const D3D12ShaderProgram&           shaderProgram,
            ID3D12RootSignature*                rootSignature,
            const GraphicsPipelineDescriptor&   desc
        );

        ComPtr<ID3D12PipelineState> pipelineState_;
        ID3D12RootSignature*        rootSignature_      = nullptr;

        D3D12_PRIMITIVE_TOPOLOGY    primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        FLOAT                       blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        UINT                        stencilRef_         = 0;

        bool                        scissorEnabled_     = false;

};


} // /namespace LLGL


#endif



// ================================================================================
