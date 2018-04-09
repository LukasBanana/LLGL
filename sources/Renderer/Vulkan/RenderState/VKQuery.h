/*
 * VKQuery.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_QUERY_H
#define LLGL_VK_QUERY_H


#include <LLGL/Query.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKQuery : public Query
{

    public:

        VKQuery(const VKPtr<VkDevice>& device, const QueryDescriptor& desc);

        // Returns the Vulkan VkQueryPool object.
        inline VkQueryPool GetVkQueryPool() const
        {
            return queryPool_.Get();
        }

    private:

        VKPtr<VkQueryPool> queryPool_;

};


} // /namespace LLGL


#endif



// ================================================================================
