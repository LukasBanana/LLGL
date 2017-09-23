/*
 * VKVertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKVertexBuffer.h"


namespace LLGL
{


VKVertexBuffer::VKVertexBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo) :
    VKBuffer { BufferType::Vertex, device, createInfo }
{
}


} // /namespace LLGL



// ================================================================================
