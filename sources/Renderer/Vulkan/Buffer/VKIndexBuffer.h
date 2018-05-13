/*
 * VKIndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_INDEX_BUFFER_H
#define LLGL_VK_INDEX_BUFFER_H


#include "VKBuffer.h"


namespace LLGL
{


class VKIndexBuffer : public VKBuffer
{

    public:

        VKIndexBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo, const IndexFormat& indexFormat);

        inline VkIndexType GetIndexType() const
        {
            return indexType_;
        }

    private:

        VkIndexType indexType_ = VK_INDEX_TYPE_UINT32;

};


} // /namespace LLGL


#endif



// ================================================================================
