/*
 * D3D12GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_GRAPHICS_PIPELINE_H__
#define __LLGL_D3D12_GRAPHICS_PIPELINE_H__


//#include "../Shader/D3D12ShaderProgram.h"
#include <LLGL/GraphicsPipeline.h>
#include "../../ComPtr.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12GraphicsPipeline : public GraphicsPipeline
{

    public:

        D3D12GraphicsPipeline(
            ID3D12Device* device,
            ID3D12RootSignature* rootSignature,
            ID3D12CommandAllocator* commandAlloc,
            const GraphicsPipelineDescriptor& desc
        );
        ~D3D12GraphicsPipeline();

        //void Bind(D3D12StateManager& stateMngr);

        inline const ComPtr<ID3D12PipelineState>& GetPipelineState() const
        {
            return pipelineState_;
        }

        inline const ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const
        {
            return commandList_;
        }

    private:

        ComPtr<ID3D12PipelineState>         pipelineState_;
        ComPtr<ID3D12GraphicsCommandList>   commandList_;

};


} // /namespace LLGL


#endif



// ================================================================================
