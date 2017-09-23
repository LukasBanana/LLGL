/*
 * VKBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BUFFER_H
#define LLGL_GL_BUFFER_H


#include <LLGL/Buffer.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKBuffer : public Buffer
{

    public:

        VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        // Returns the hardware buffer object.
        inline VkBuffer Get() const
        {
            return buffer_.Get();
        }

        // Returns the memory requirements of the hardware buffer.
        inline const VkMemoryRequirements& GetRequirements() const
        {
            return requirements_;
        }

    private:

        VKPtr<VkBuffer>         buffer_;
        VkMemoryRequirements    requirements_;

};


} // /namespace LLGL


#endif



// ================================================================================
