/*
 * BindingDescriptorIterator.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BINDING_DESCRIPTOR_ITERATOR_H
#define LLGL_BINDING_DESCRIPTOR_ITERATOR_H


#include <LLGL/Export.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/ForwardDecls.h>


namespace LLGL
{


// Helper class to iterate over all resource views and their binding points of a certain type
class LLGL_EXPORT BindingDescriptorIterator
{

    public:

        BindingDescriptorIterator(const ArrayView<BindingDescriptor>& bindings, std::size_t offset = 0, std::size_t count = 0);

        // Resets the iteration for the specified binding parameters.
        void Reset(const ResourceType typeOfInterest, long bindFlagsOfInterest = 0, long stagesOfInterest = 0);

        // Returns the next binding descriptor, or null if there are no descriptors with the active filter.
        const BindingDescriptor* Next(std::size_t* outIndex = nullptr);

        // Returns the number of all resource.
        inline std::size_t GetCount() const
        {
            return count_;
        }

    private:

        ArrayView<BindingDescriptor>    bindings_;
        std::size_t                     iterator_               = 0;
        std::size_t                     offset_                 = 0;
        std::size_t                     count_                  = 0;
        ResourceType                    typeOfInterest_         = ResourceType::Undefined;
        long                            bindFlagsOfInterest_    = ~0;
        long                            stagesOfInterest_       = StageFlags::AllStages;

};


// Returns the specified resource as Buffer and throws an excpetion if the type does not match or a null pointer is passed.
LLGL_EXPORT Buffer* GetAsExpectedBuffer(Resource* resource, long anyBindFlags = 0);
LLGL_EXPORT Texture* GetAsExpectedTexture(Resource* resource, long anyBindFlags = 0);
LLGL_EXPORT Sampler* GetAsExpectedSampler(Resource* resource);


} // /namespace LLGL


#endif



// ================================================================================
