/*
 * ResourceBindingIterator.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_BINDING_ITERATOR_H
#define LLGL_RESOURCE_BINDING_ITERATOR_H


#include <LLGL/Export.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <vector>


namespace LLGL
{


// Helper class to iterate over a all resource views and theirs bindings points of a certain type
class LLGL_EXPORT ResourceBindingIterator
{

    public:

        ResourceBindingIterator(
            const std::vector<ResourceViewDescriptor>&  resourceViews,
            const std::vector<BindingDescriptor>&       bindings
        );

        // Resets the iteration process for the specified type of interest.
        void Reset(const ResourceType typesOfInterest);

        // Returns the next resource of the current type of interest, or null if there are no more resources of that type.
        Resource* Next(BindingDescriptor& bindingDesc);

        // Returns the number of all resource.
        inline std::size_t GetCount() const
        {
            return count_;
        }

    private:

        const std::vector<ResourceViewDescriptor>&  resourceViews_;
        const std::vector<BindingDescriptor>&       bindings_;
        std::size_t                                 iterator_       = 0;
        std::size_t                                 count_          = 0;
        ResourceType                                typeOfInterest_ = ResourceType::Undefined;

};


} // /namespace LLGL


#endif



// ================================================================================
