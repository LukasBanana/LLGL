/*
 * VKBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_BUFFER_H
#define LLGL_VK_BUFFER_H


#include <LLGL/Buffer.h>
#include "VKDeviceBuffer.h"
#include "../Memory/VKDeviceMemory.h"


namespace LLGL
{


class VKBuffer : public Buffer
{

    public:

        VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);
        void TakeStagingBuffer(VKDeviceBuffer&& deviceBuffer);

        void* Map(VkDevice device, const CPUAccess access);
        void Unmap(VkDevice device);

        void* MapStaging(VkDevice device, VkDeviceSize dataSize, VkDeviceSize offset = 0);
        void UnmapStaging(VkDevice device);

        // Returns the device buffer object.
        inline VKDeviceBuffer& GetDeviceBuffer()
        {
            return bufferObj_;
        }

        // Returns the staging device buffer object.
        inline VKDeviceBuffer& GetStagingDeviceBuffer()
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

        // Returns the CPU access previously set when "Map" was called.
        inline CPUAccess GetMappingCPUAccess() const
        {
            return mappingCPUAccess_;
        }

    private:

        VKDeviceBuffer  bufferObj_;
        VKDeviceBuffer  bufferObjStaging_;

        VkDeviceSize    size_               = 0;
        CPUAccess       mappingCPUAccess_   = CPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
