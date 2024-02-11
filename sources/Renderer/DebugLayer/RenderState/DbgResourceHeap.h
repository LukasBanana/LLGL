/*
 * DbgResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_RESOURCE_HEAP_H
#define LLGL_DBG_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <string>


namespace LLGL
{


class DbgResourceHeap final : public ResourceHeap
{

    public:

        void SetDebugName(const char* name) override;

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        DbgResourceHeap(ResourceHeap& instance, const ResourceHeapDescriptor& desc);

        // Returns the number of descriptor sets by using the debug information only, i.e. 'desc.numResourceViews' and 'numBindings'.
        std::uint32_t GetNumDescriptorSetsSafe() const;

    public:

        ResourceHeap&                   instance;
        const ResourceHeapDescriptor    desc;
        std::string                     label;
        const std::uint32_t             numBindings = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
