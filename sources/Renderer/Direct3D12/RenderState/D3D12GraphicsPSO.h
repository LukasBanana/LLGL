/*
 * D3D12GraphicsPSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_GRAPHICS_PIPELINE_H
#define LLGL_D3D12_GRAPHICS_PIPELINE_H


#include "D3D12PipelineState.h"
#include "../../Serialization.h"


namespace LLGL
{


class D3D12Device;
class D3D12RenderPass;
class D3D12PipelineLayout;
class D3D12ShaderProgram;
class D3D12CommandContext;
class ByteBufferIterator;

class D3D12GraphicsPSO final : public D3D12PipelineState
{

    public:

        // Constructs the graphics PSO with the specified descriptor.
        D3D12GraphicsPSO(
            D3D12Device&                        device,
            D3D12PipelineLayout&                defaultPipelineLayout,
            const GraphicsPipelineDescriptor&   desc,
            const D3D12RenderPass*              defaultRenderPass,
            Serialization::Serializer*          writer                  = nullptr
        );

        // Constructs the graphics PSO with a deserializer of a cached PSO.
        D3D12GraphicsPSO(D3D12Device& device, Serialization::Deserializer& reader);

        // Binds this graphics PSO to the specified command context.
        void Bind(D3D12CommandContext& commandContext) override;

        // Returns the number of required default scissor rectangles.
        UINT NumDefaultScissorRects() const;

        // Returns true if scissors are enabled.
        inline bool IsScissorEnabled() const
        {
            return scissorEnabled_;
        }

    private:

        void CreateNativePSOFromDesc(
            D3D12Device&                        device,
            const D3D12PipelineLayout&          pipelineLayout,
            const D3D12ShaderProgram&           shaderProgram,
            const D3D12RenderPass*              renderPass,
            const GraphicsPipelineDescriptor&   desc,
            Serialization::Serializer*          writer
        );

        void CreateNativePSOFromCache(
            D3D12Device&                    device,
            Serialization::Deserializer&    reader
        );

        void SerializePSO(
            Serialization::Serializer&                  writer,
            const D3D12_GRAPHICS_PIPELINE_STATE_DESC&   stateDesc,
            ID3DBlob*                                   rootSignatureBlob,
            ID3DBlob*                                   psoCacheBlob
        );

        void DeserializePSO(
            Serialization::Deserializer&                reader,
            D3D12_GRAPHICS_PIPELINE_STATE_DESC&         stateDesc,
            std::vector<D3D12_INPUT_ELEMENT_DESC>&      inputElements,
            std::vector<D3D12_SO_DECLARATION_ENTRY>&    soDeclEntries,
            std::vector<UINT>&                          soBufferStrides
        );

        void BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);

        void SetStaticViewportsAndScissors(ID3D12GraphicsCommandList* commandList);

    private:

        D3D12_PRIMITIVE_TOPOLOGY    primitiveTopology_  = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        bool                        scissorEnabled_     = false;

        bool                        stencilRefEnabled_  = false;
        UINT                        stencilRef_         = 0;

        bool                        blendFactorEnabled_ = false;
        FLOAT                       blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };

        std::unique_ptr<char[]>     staticStateBuffer_;
        UINT                        numStaticViewports_ = 0;
        UINT                        numStaticScissors_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
