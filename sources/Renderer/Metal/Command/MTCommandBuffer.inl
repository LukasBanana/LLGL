/*
 * MTCommandBuffer.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Backend/CommandBuffer.Encoding.inl>
#include <LLGL/Backend/CommandBuffer.Blitting.inl>
#include <LLGL/Backend/CommandBuffer.ViewportsAndScissors.inl>
/*exclude<LLGL/Backend/CommandBuffer.InputAssembly.inl>*/
/*exclude<LLGL/Backend/CommandBuffer.Resources.inl>*/
#include <LLGL/Backend/CommandBuffer.RenderPasses.inl>
/*exclude<LLGL/Backend/CommandBuffer.PipelineStates.inl>*/
#include <LLGL/Backend/CommandBuffer.Queries.inl>
#include <LLGL/Backend/CommandBuffer.StreamOutput.inl>
#include <LLGL/Backend/CommandBuffer.Drawing.inl>
#include <LLGL/Backend/CommandBuffer.Compute.inl>
#include <LLGL/Backend/CommandBuffer.Debugging.inl>
/*exclude<LLGL/Backend/CommandBuffer.Extensions.inl>*/


/* ----- Input Assembly ------ */

virtual void SetVertexBuffer(
    Buffer&             buffer
) override final;

virtual void SetVertexBufferArray(
    BufferArray&        bufferArray
) override final;

/* ----- Resources ----- */

virtual void SetResourceHeap(
    ResourceHeap&       resourceHeap,
    std::uint32_t       descriptorSet   = 0
) override final;

virtual void ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags      = StageFlags::AllStages
) override final;

/* ----- Pipeline States ----- */

virtual void SetPipelineState(
    PipelineState&      pipelineState
) override final;

virtual void SetBlendFactor(
    const float         color[4]
) override final;

virtual void SetStencilReference(
    std::uint32_t       reference,
    const StencilFace   stencilFace     = StencilFace::FrontAndBack
) override final;

/* ----- Extensions ----- */

virtual bool GetNativeHandle(
    void*               nativeHandle,
    std::size_t         nativeHandleSize
) override final;



// ================================================================================
