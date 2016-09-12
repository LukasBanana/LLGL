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


class D3D12RenderSystem;

class D3D12GraphicsPipeline : public GraphicsPipeline
{

    public:

        D3D12GraphicsPipeline(
            D3D12RenderSystem& renderSystem,
            ID3D12RootSignature* rootSignature,
            const GraphicsPipelineDescriptor& desc
        );

        //void Bind(D3D12StateManager& stateMngr);

        inline ID3D12PipelineState* GetPipelineState() const
        {
            return pipelineState_.Get();
        }

    private:

        ComPtr<ID3D12PipelineState> pipelineState_;

};


} // /namespace LLGL


#endif



// ================================================================================
