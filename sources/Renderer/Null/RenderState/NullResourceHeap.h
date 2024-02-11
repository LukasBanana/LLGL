/*
 * NullResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_RESOURCE_HEAP_H
#define LLGL_NULL_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <string>
#include <vector>


namespace LLGL
{


class NullResourceHeap final : public ResourceHeap
{

    public:

        void SetDebugName(const char* name) override;

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        NullResourceHeap(const ResourceHeapDescriptor& desc, const ArrayView<ResourceViewDescriptor>& initialResourceViews = {});

        std::uint32_t WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews);

    private:

        std::string                         label_;
        const std::uint32_t                 numBindings_    = 1;
        std::vector<ResourceViewDescriptor> resourceViews_;

};


} // /namespace LLGL


#endif



// ================================================================================
