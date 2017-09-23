/*
 * VKVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_VERTEX_BUFFER_H
#define LLGL_GL_VERTEX_BUFFER_H


#include "VKBuffer.h"


namespace LLGL
{


class VKVertexBuffer : public VKBuffer
{

    public:

        VKVertexBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);



    private:

};


} // /namespace LLGL


#endif



// ================================================================================
