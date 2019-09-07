/*
 * D3D12GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_GRAPHICS_PIPELINE_H
#define LLGL_D3D12_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>
#include <LLGL/ForwardDecls.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <memory>


namespace LLGL
{


class D3D12Device;
class D3D12RenderPass;
class D3D12ShaderProgram;
class D3D12CommandContext;
class ByteBufferIterator;

class D3D12GraphicsPipeline final : public GraphicsPipeline
{

    public:

        void SetName(const char* name) override;

    public:

        D3D12GraphicsPipeline(
            D3D12Device&                        device,
            ID3D12RootSignature*                defaultRootSignature,
            const GraphicsPipelineDescriptor&   desc
        );

        void Bind(D3D12CommandContext& commandContext);

        // Returns the number of required default scissor rectangles.
        UINT NumDefaultScissorRects() const;

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
            const D3D12RenderPass*              renderPass,
            const GraphicsPipelineDescriptor&   desc
        );

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);
        void SetStaticViewportsAndScissors(ID3D12GraphicsCommandList* commandList);

    private:

        ComPtr<ID3D12PipelineState> pipelineState_;
        ID3D12RootSignature*        rootSignature_      = nullptr;

        D3D12_PRIMITIVE_TOPOLOGY    primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        FLOAT                       blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };
        UINT                        stencilRef_         = 0;

        bool                        scissorEnabled_     = false;

        std::unique_ptr<char[]>     staticStateBuffer_;
        UINT                        numStaticViewports_ = 0;
        UINT                        numStaticScissors_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
