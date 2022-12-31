/*
 * NullResourceHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_RESOURCE_HEAP_H
#define LLGL_NULL_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <string>


namespace LLGL
{


class NullResourceHeap final : public ResourceHeap
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        NullResourceHeap(const ResourceHeapDescriptor& desc);

    public:

        const ResourceHeapDescriptor desc;

    private:

        std::string         label_;
        const std::uint32_t numBindings_ = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
