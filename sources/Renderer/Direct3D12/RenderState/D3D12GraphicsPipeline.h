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


class D3D12RenderSystem;
class D3D12ShaderProgram;

class D3D12GraphicsPipeline : public GraphicsPipeline
{

    public:

        D3D12GraphicsPipeline(
            D3D12RenderSystem& renderSystem,
            //ID3D12RootSignature* rootSignature,
            const GraphicsPipelineDescriptor& desc
        );

        inline ID3D12RootSignature* GetRootSignature() const
        {
            return rootSignature_.Get();
        }

        inline ID3D12PipelineState* GetPipelineState() const
        {
            return pipelineState_.Get();
        }

        inline D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
        {
            return primitiveTopology_;
        }

    private:

        void CreateRootSignature(D3D12RenderSystem& renderSystem, D3D12ShaderProgram& shaderProgram, const GraphicsPipelineDescriptor& desc);
        void CreatePipelineState(D3D12RenderSystem& renderSystem, D3D12ShaderProgram& shaderProgram, const GraphicsPipelineDescriptor& desc);

        ComPtr<ID3D12RootSignature> rootSignature_;
        ComPtr<ID3D12PipelineState> pipelineState_;

        D3D12_PRIMITIVE_TOPOLOGY    primitiveTopology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

};


} // /namespace LLGL


#endif



// ================================================================================
