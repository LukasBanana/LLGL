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

        // Returns the internal ID3D12RootSignature object.
        inline ID3D12RootSignature* GetRootSignature() const
        {
            return rootSignature_;
        }

        // Returns the internal ID3D12PipelineState object.
        inline ID3D12PipelineState* GetPipelineState() const
        {
            return pipelineState_.Get();
        }

        // Returns the primitive topology.
        inline D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
        {
            return primitiveTopology_;
        }

        // Returns true if scissors are enabled.
        inline bool IsScissorEnabled() const
        {
            return scissorEnabled_;
        }

    private:

        void CreateDefaultRootSignature(ID3D12Device* device);

        void CreatePipelineState(
            D3D12RenderSystem&                  renderSystem,
            D3D12ShaderProgram&                 shaderProgram,
            ID3D12RootSignature*                rootSignature,
            const GraphicsPipelineDescriptor&   desc
        );

        ComPtr<ID3D12PipelineState> pipelineState_;
        ID3D12RootSignature*        rootSignature_      = nullptr;

        D3D12_PRIMITIVE_TOPOLOGY    primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        bool                        scissorEnabled_     = false;

        #if 1//TODO: replace this by D3D12PipelineLayout
        ComPtr<ID3D12RootSignature> defaultRootSignature_;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
