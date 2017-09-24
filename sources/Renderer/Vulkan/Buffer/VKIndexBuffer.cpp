/*
 * VKIndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKIndexBuffer.h"
#include "../VKTypes.h"


namespace LLGL
{


static VkIndexType MapIndexType(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::UInt16:  return VK_INDEX_TYPE_UINT16;
        case DataType::UInt32:  return VK_INDEX_TYPE_UINT32;
        default:                break;
    }
    VKTypes::MapFailed("DataType", "VkIndexType");
}

VKIndexBuffer::VKIndexBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo, const IndexFormat& indexFormat) :
    VKBuffer   { BufferType::Index, device, createInfo   },
    indexType_ { MapIndexType(indexFormat.GetDataType()) }
{
}


} // /namespace LLGL



// ================================================================================
