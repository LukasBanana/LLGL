/*
 * BindingIterator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BINDING_ITERATOR_H
#define LLGL_BINDING_ITERATOR_H


#include <LLGL/Export.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/ForwardDecls.h>


namespace LLGL
{


/*
Helper class to iterate over all resource views and their binding points of a certain type.
TBinding must have the following fields: type (ResourceType), bindFlags (long), stageFlags (long).
*/
template <typename TBinding>
class LLGL_EXPORT BindingIterator
{

    public:

        BindingIterator(const ArrayView<TBinding>& bindings) :
            bindings_ { bindings }
        {
        }

        // Resets the iteration for the specified binding parameters.
        void Reset(const ResourceType typeOfInterest, long bindFlagsOfInterest = 0, long stagesOfInterest = 0)
        {
            iterator_               = 0;
            typeOfInterest_         = typeOfInterest;
            bindFlagsOfInterest_    = bindFlagsOfInterest;
            stagesOfInterest_       = stagesOfInterest;
        }

        // Returns the next binding descriptor, or null if there are no descriptors with the active filter.
        const TBinding* Next(std::size_t* outIndex = nullptr)
        {
            while (iterator_ < bindings_.size())
            {
                /* Search for resource type of interest */
                std::size_t index = iterator_++;
                const TBinding& binding = bindings_[index % bindings_.size()];
                if ( binding.type == typeOfInterest_ &&
                    ( bindFlagsOfInterest_ == 0 || (binding.bindFlags  & bindFlagsOfInterest_) != 0 ) &&
                    ( stagesOfInterest_    == 0 || (binding.stageFlags & stagesOfInterest_   ) != 0 ) )
                {
                    /* Return binding descriptor and optional index */
                    if (outIndex != nullptr)
                        *outIndex = index;
                    return (&binding);
                }
            }
            return nullptr;
        }

        // Returns the number of bindings this iterator refers to.
        std::size_t GetCount() const
        {
            return bindings_.size();
        }

    private:

        ArrayView<TBinding> bindings_;
        std::size_t         iterator_               = 0;
        ResourceType        typeOfInterest_         = ResourceType::Undefined;
        long                bindFlagsOfInterest_    = ~0;
        long                stagesOfInterest_       = StageFlags::AllStages;

};

using BindingDescriptorIterator = BindingIterator<BindingDescriptor>;


// Returns the specified resource as Buffer and throws an excpetion if the type does not match or a null pointer is passed.
LLGL_EXPORT Buffer* GetAsExpectedBuffer(Resource* resource, long anyBindFlags = 0);
LLGL_EXPORT Texture* GetAsExpectedTexture(Resource* resource, long anyBindFlags = 0);
LLGL_EXPORT Sampler* GetAsExpectedSampler(Resource* resource);


} // /namespace LLGL


#endif



// ================================================================================
