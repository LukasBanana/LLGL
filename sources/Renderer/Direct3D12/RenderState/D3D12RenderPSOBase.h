/*
 * D3D12RenderPSOBase.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RENDER_PIPELINE_BASE_H
#define LLGL_D3D12_RENDER_PIPELINE_BASE_H


#include "D3D12PipelineState.h"
#include <LLGL/Container/DynamicArray.h>


namespace LLGL
{


class ByteBufferIterator;
class PipelineCache;
class D3D12Device;
class D3D12RenderPass;
class D3D12PipelineLayout;
class D3D12CommandContext;
class D3D12PipelineCache;

class D3D12RenderPSOBase : public D3D12PipelineState
{

    public:

        // Constructs the graphics PSO with the specified descriptor.
        D3D12RenderPSOBase(
            D3D12PipelineType       type,

            const StencilDescriptor&    stencilDesc,
            const BlendDescriptor&      blendDesc,
            bool                        isScissorEnabled,
            const ArrayView<Viewport>&  staticViewports,
            const ArrayView<Scissor>&   staticScissors,

            const PipelineLayout*       pipelineLayout,
            const ArrayView<Shader*>&   shaders,
            D3D12PipelineLayout&        defaultPipelineLayout
        );

        // Returns the number of required default scissor rectangles.
        UINT NumDefaultScissorRects() const;

        // Returns true if scissors are enabled.
        inline bool IsScissorEnabled() const
        {
            return (scissorEnabled_ != 0);
        }

    protected:

        void BindOutputMergerAndStaticStates(ID3D12GraphicsCommandList* commandList);

    private:

        void BuildStaticStateBuffer(const ArrayView<Viewport>& staticViewports, const ArrayView<Scissor>& staticScissors);
        void BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter);
        void BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter);

        void SetStaticViewportsAndScissors(ID3D12GraphicsCommandList* commandList);

    private:

        UINT                scissorEnabled_     : 1;

        UINT                stencilRefEnabled_  : 1;
        UINT                stencilRef_         : 8;

        UINT                blendFactorEnabled_ : 1;
        FLOAT               blendFactor_[4]     = { 0.0f, 0.0f, 0.0f, 0.0f };

        DynamicByteArray    staticStateBuffer_;
        UINT                numStaticViewports_ = 0;
        UINT                numStaticScissors_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
