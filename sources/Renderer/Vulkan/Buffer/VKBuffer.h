/*
 * VKBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_BUFFER_H
#define LLGL_VK_BUFFER_H


#include <LLGL/Buffer.h>
#include "../Memory/VKDeviceMemory.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <memory>


namespace LLGL
{


struct VKBufferWithRequirements
{
    VKBufferWithRequirements(const VKPtr<VkDevice>& device);
    VKBufferWithRequirements(VKBufferWithRequirements&& rhs);
    VKBufferWithRequirements& operator = (VKBufferWithRequirements&& rhs);

    void Create(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);
    void Release();

    VKPtr<VkBuffer>         buffer;
    VkMemoryRequirements    requirements;
};


class VKBuffer : public Buffer
{

    public:

        VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        void BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion);
        void TakeStagingBuffer(VKBufferWithRequirements&& buffer, VKDeviceMemoryRegion* memoryRegionStaging);

        void* Map(VkDevice device, const BufferCPUAccess access);
        void Unmap(VkDevice device);

        // Updates the staging buffer (if it was created).
        void UpdateStagingBuffer(VkDevice device, const void* data, VkDeviceSize dataSize, VkDeviceSize offset = 0);

        // Returns the hardware buffer object.
        inline VkBuffer GetVkBuffer() const
        {
            return bufferObj_.buffer.Get();
        }

        // Returns the hardware staging buffer object.
        inline VkBuffer GetStagingVkBuffer() const
        {
            return bufferObjStaging_.buffer.Get();
        }

        // Returns the memory requirements of the hardware buffer.
        inline const VkMemoryRequirements& GetRequirements() const
        {
            return bufferObj_.requirements;
        }

        // Returns the size originally specified in the descriptor.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

        // Returns the CPU access previously set when "Map" was called.
        inline BufferCPUAccess GetMappingCPUAccess() const
        {
            return mappingCPUAccess_;
        }

    private:

        VKBufferWithRequirements    bufferObj_;
        VKDeviceMemoryRegion*       memoryRegion_           = nullptr;

        VKBufferWithRequirements    bufferObjStaging_;
        VKDeviceMemoryRegion*       memoryRegionStaging_    = nullptr;

        VkDeviceSize                size_                   = 0;
        BufferCPUAccess             mappingCPUAccess_       = BufferCPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
