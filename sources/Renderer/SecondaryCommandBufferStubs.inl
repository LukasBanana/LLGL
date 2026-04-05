/*
 * SecondaryCommandBufferStubs.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SECONDARY_COMMAND_BUFFER_CLASS
#error Missing definition of LLGL_SECONDARY_COMMAND_BUFFER_CLASS in SecondaryCommandBufferStubs.h
#endif


/* ----- Encoding ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::Execute(
    LLGL::CommandBuffer& /*secondaryCommandBuffer*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Blitting ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::UpdateBuffer(
    LLGL::Buffer&   /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    const void*     /*data*/,
    std::uint64_t   /*dataSize*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::CopyBuffer(
    LLGL::Buffer&   /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    LLGL::Buffer&   /*srcBuffer*/,
    std::uint64_t   /*srcOffset*/,
    std::uint64_t   /*size*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::CopyBufferFromTexture(
    LLGL::Buffer&               /*dstBuffer*/,
    std::uint64_t               /*dstOffset*/,
    LLGL::Texture&              /*srcTexture*/,
    const LLGL::TextureRegion&  /*srcRegion*/,
    std::uint32_t               /*rowStride*/,
    std::uint32_t               /*layerStride*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::FillBuffer(
    LLGL::Buffer&   /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    std::uint32_t   /*value*/,
    std::uint64_t   /*fillSize*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::CopyTexture(
    LLGL::Texture&                  /*dstTexture*/,
    const LLGL::TextureLocation&    /*dstLocation*/,
    LLGL::Texture&                  /*srcTexture*/,
    const LLGL::TextureLocation&    /*srcLocation*/,
    const LLGL::Extent3D&           /*extent*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::CopyTextureFromBuffer(
    LLGL::Texture&              /*dstTexture*/,
    const LLGL::TextureRegion&  /*dstRegion*/,
    LLGL::Buffer&               /*srcBuffer*/,
    std::uint64_t               /*srcOffset*/,
    std::uint32_t               /*rowStride*/,
    std::uint32_t               /*layerStride*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::CopyTextureFromFramebuffer(
    LLGL::Texture&              /*dstTexture*/,
    const LLGL::TextureRegion&  /*dstRegion*/,
    const LLGL::Offset2D&       /*srcOffset*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::GenerateMips(
    LLGL::Texture& /*texture*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::GenerateMips(
    LLGL::Texture&                  /*texture*/,
    const LLGL::TextureSubresource& /*subresource*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Viewport and Scissor ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::SetViewport(
    const LLGL::Viewport& /*viewport*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::SetViewports(
    std::uint32_t           /*numViewports*/,
    const LLGL::Viewport*   /*viewports*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::SetScissor(
    const LLGL::Scissor& /*scissor*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::SetScissors(
    std::uint32_t           /*numScissors*/,
    const LLGL::Scissor*    /*scissors*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Render Passes ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::BeginRenderPass(
    LLGL::RenderTarget&     /*renderTarget*/,
    const LLGL::RenderPass* /*renderPass*/,
    std::uint32_t           /*numClearValues*/,
    const LLGL::ClearValue* /*clearValues*/,
    std::uint32_t           /*swapBufferIndex*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::EndRenderPass()
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::Clear(
    long                    /*flags*/,
    const LLGL::ClearValue& /*clearValue*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::ClearAttachments(
    std::uint32_t                   /*numAttachments*/,
    const LLGL::AttachmentClear*    /*attachments*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Queries ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::BeginQuery(
    LLGL::QueryHeap&    /*queryHeap*/,
    std::uint32_t       /*query*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::EndQuery(
    LLGL::QueryHeap&    /*queryHeap*/,
    std::uint32_t       /*query*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::BeginRenderCondition(
    LLGL::QueryHeap&                /*queryHeap*/,
    std::uint32_t                   /*query*/,
    const LLGL::RenderConditionMode /*mode*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::EndRenderCondition()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Stream Output ------ */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::BeginStreamOutput(
    std::uint32_t           /*numBuffers*/,
    LLGL::Buffer* const *   /*buffers*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::EndStreamOutput()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Debugging ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::PushDebugGroup(
    const char* /*name*/)
{
    // dummy - command not allowed in secondary command buffer
}

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::PopDebugGroup()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Extensions ----- */

void LLGL_SECONDARY_COMMAND_BUFFER_CLASS::DoNativeCommand(
    const void* /*nativeCommand*/,
    std::size_t /*nativeCommandSize*/)
{
    // dummy - command not allowed in secondary command buffer
}

bool LLGL_SECONDARY_COMMAND_BUFFER_CLASS::GetNativeHandle(
    void*       /*nativeHandle*/,
    std::size_t /*nativeHandleSize*/)
{
    return false; // dummy - command not allowed in secondary command buffer
}



// ================================================================================
