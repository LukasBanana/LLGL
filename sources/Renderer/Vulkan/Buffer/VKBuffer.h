/*
 * VKBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_BUFFER_H
#define LLGL_VK_BUFFER_H


#include <LLGL/Buffer.h>
#include "VKDeviceBuffer.h"
#include "../Memory/VKDeviceMemory.h"


namespace LLGL
{


class VKDevice;

class VKBuffer : public Buffer
{

    public:

        BufferDescriptor GetDesc() const override;

    public:

        VKBuffer(const VKPtr<VkDevice>& device, const BufferDescriptor& desc);

        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);
        void TakeStagingBuffer(VKDeviceBuffer&& deviceBuffer);

        void* Map(VKDevice& device, const CPUAccess access, VkDeviceSize offset, VkDeviceSize length);
        void Unmap(VKDevice& device);

        // Returns the device buffer object.
        inline VKDeviceBuffer& GetDeviceBuffer()
        {
            return bufferObj_;
        }

        // Returns the device buffer object as constant reference.
        inline const VKDeviceBuffer& GetDeviceBuffer() const
        {
            return bufferObj_;
        }

        // Returns the staging device buffer object.
        inline VKDeviceBuffer& GetStagingDeviceBuffer()
        {
            return bufferObjStaging_;
        }

        // Returns the staging device buffer object as constant reference.
        inline const VKDeviceBuffer& GetStagingDeviceBuffer() const
        {
            return bufferObjStaging_;
        }

        // Returns the hardware buffer object.
        inline VkBuffer GetVkBuffer() const
        {
            return bufferObj_.GetVkBuffer();
        }

        // Returns the hardware staging buffer object.
        inline VkBuffer GetStagingVkBuffer() const
        {
            return bufferObjStaging_.GetVkBuffer();
        }

        // Returns the size originally specified in the descriptor.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

        // Returns the VkIndexType specified at creation time.
        inline VkIndexType GetIndexType() const
        {
            return indexType_;
        }

    private:

        VKDeviceBuffer  bufferObj_;
        VKDeviceBuffer  bufferObjStaging_;

        VkDeviceSize    size_                   = 0;
        VkDeviceSize    mappedWriteRange_[2]    = { 0, 0 };

        VkIndexType     indexType_              = VK_INDEX_TYPE_MAX_ENUM;

};


} // /namespace LLGL


#endif



// ================================================================================
