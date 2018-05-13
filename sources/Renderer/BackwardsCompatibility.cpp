/*
 * BackwardsCompatibility.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_BACKWARDS_COMPATIBILITY

#include <LLGL/RenderSystem.h>
#include <LLGL/CommandBuffer.h>
#include <LLGL/ShaderProgram.h>


namespace LLGL
{


/* ----- RenderSystem ------ */

TextureDescriptor RenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    return texture.QueryDesc();
}

void RenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    const std::size_t dataSize = TextureSize(texture.QueryDesc()) * ImageFormatSize(imageFormat) * DataTypeSize(dataType);
    ReadTexture(texture, mipLevel, imageFormat, dataType, buffer, dataSize);
}


/* ----- CommandBuffer ----- */

void CommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    SetViewports(numViewports, viewportArray);
}

void CommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
{
    SetScissors(numScissors, scissorArray);
}

void CommandBuffer::ClearTarget(unsigned int targetIndex, const LLGL::ColorRGBAf& color)
{
    AttachmentClear attachment;
    {
        attachment.flags            = ClearFlags::Color;
        attachment.colorAttachment  = targetIndex;
        attachment.clearValue.color = color;
    }
    ClearAttachments(1, &attachment);
}

void CommandBuffer::SyncGPU()
{
    //todo
}


/* ----- ShaderProgram ----- */

void ShaderProgram::BuildInputLayout(const VertexFormat& vertexFormat)
{
    BuildInputLayout(1, &vertexFormat);
}


} // /namespace LLGL

#endif // /LLGL_ENABLE_BACKWARDS_COMPATIBILITY



// ================================================================================
