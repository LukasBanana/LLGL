/*
 * VKBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        #include <LLGL/Backend/Buffer.inl>

    public:

        VKBuffer(VkDevice device, const BufferDescriptor& desc);

        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);
        void TakeStagingBuffer(VKDeviceBuffer&& deviceBuffer);

        void* Map(VKDevice& device, const CPUAccess access, VkDeviceSize offset, VkDeviceSize length);
        void Unmap(VKDevice& device);

        // Returns the actual size of this buffer.
        // This might be larger than GetSize() if the buffer has additional payload such as the transform-feedback counter.
        VkDeviceSize GetInternalSize() const;

        // Returns the offset to the transform-feedback counter within this buffer or 0 if there is no such counter.
        VkDeviceSize GetXfbCounterOffset() const;

        // Creates a VkBufferView for this buffer. If this buffer was not created with a valid format, the return value is false.
        bool CreateBufferView(VkDevice device, VKPtr<VkBufferView>& outBufferView, VkDeviceSize offset = 0, VkDeviceSize length = VK_WHOLE_SIZE);

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

        // Returns the VkAccessFlags bitmask used for pipeline barriers.
        inline VkAccessFlags GetAccessFlags() const
        {
            return accessFlags_;
        }

        // Returns true element stride this buffer was created with. Currently only used for vertex buffers.
        inline std::uint32_t GetStride() const
        {
            return stride_;
        }

        // Returns a pointer to the VkBufferView object or null if there is none.
        inline VkBufferView GetBufferView() const
        {
            return bufferView_.Get();
        }

    private:

        VKDeviceBuffer      bufferObj_;
        VKDeviceBuffer      bufferObjStaging_;

        VKPtr<VkBufferView> bufferView_;

        VkDeviceSize        size_                   = 0;
        VkDeviceSize        mappedWriteRange_[2]    = { 0, 0 };

        VkIndexType         indexType_              = VK_INDEX_TYPE_MAX_ENUM;

        VkAccessFlags       accessFlags_            = 0;
        VkFormat            format_                 = VK_FORMAT_UNDEFINED;
        std::uint32_t       stride_                 = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
