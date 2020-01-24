/*
 * ResourceBindingIterator.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ResourceBindingIterator.h"
#include <algorithm>


namespace LLGL
{


ResourceBindingIterator::ResourceBindingIterator(
    const std::vector<ResourceViewDescriptor>&  resourceViews,
    const std::vector<BindingDescriptor>&       bindings,
    std::size_t                                 firstResourceIndex,
    bool                                        iterateAllSegments)
:
    resourceViews_ { resourceViews        },
    bindings_      { bindings             },
    offset_        { firstResourceIndex   },
    count_         { resourceViews.size() }
{
    if (!iterateAllSegments)
        count_ = std::min(count_, bindings.size());
}

void ResourceBindingIterator::Reset(const ResourceType typeOfInterest, long bindFlagsOfInterest, long stagesOfInterest)
{
    iterator_               = 0;
    typeOfInterest_         = typeOfInterest;
    bindFlagsOfInterest_    = bindFlagsOfInterest;
    stagesOfInterest_       = stagesOfInterest;
}

// Returns the specified resource type as string
static const char* ResourceTypeToString(const ResourceType t)
{
    switch (t)
    {
        case ResourceType::Buffer:  return "Buffer";
        case ResourceType::Texture: return "Texture";
        case ResourceType::Sampler: return "Sampler";
        default:                    return "Undefined";
    }
}

[[noreturn]]
static void ErrNullPointerResource(const ResourceType t)
{
    throw std::invalid_argument(
        "null pointer exception of resource object used as binding point for 'LLGL::ResourceType::"
        + std::string(ResourceTypeToString(t)) + "'"
    );
}

Resource* ResourceBindingIterator::Next(const BindingDescriptor** bindingDesc, const ResourceViewDescriptor** rvDesc)
{
    while (iterator_ < count_)
    {
        /* Search for resource type of interest */
        const auto& current = bindings_[iterator_ % bindings_.size()];
        if ( current.type == typeOfInterest_ &&
             ( bindFlagsOfInterest_ == 0 || (current.bindFlags  & bindFlagsOfInterest_) != 0 ) &&
             ( stagesOfInterest_    == 0 || (current.stageFlags & stagesOfInterest_   ) != 0 ) )
        {
            /* Check for null pointer exception */
            const auto& resourceViewDesc = resourceViews_[offset_ + iterator_];
            if (auto resource = resourceViewDesc.resource)
            {
                if (bindingDesc != nullptr)
                    *bindingDesc = &current;
                if (rvDesc != nullptr)
                    *rvDesc = &resourceViewDesc;
                ++iterator_;
                return resource;
            }
            else
                ErrNullPointerResource(current.type);
        }
        ++iterator_;
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
