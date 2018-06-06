/*
 * ResourceBindingIterator.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ResourceBindingIterator.h"
#include <algorithm>


namespace LLGL
{


ResourceBindingIterator::ResourceBindingIterator(
    const std::vector<ResourceViewDescriptor>& resourceViews,
    const std::vector<BindingDescriptor>& bindings) :
        resourceViews_ { resourceViews                                   },
        bindings_      { bindings                                        },
        count_         { std::min(resourceViews.size(), bindings.size()) }
{
}

void ResourceBindingIterator::Reset(const ResourceType typesOfInterest, long stagesOfInterest)
{
    iterator_           = 0;
    typeOfInterest_     = typesOfInterest;
    stagesOfInterest_   = stagesOfInterest;
}

// Returns the specified resource type as string
static const char* ResourceTypeToString(const ResourceType t)
{
    switch (t)
    {
        case ResourceType::VertexBuffer:        return "VertexBuffer";
        case ResourceType::IndexBuffer:         return "IndexBuffer";
        case ResourceType::ConstantBuffer:      return "ConstantBuffer";
        case ResourceType::StorageBuffer:       return "StorageBuffer";
        case ResourceType::StreamOutputBuffer:  return "StreamOutputBuffer";
        case ResourceType::Texture:             return "Texture";
        case ResourceType::Sampler:             return "Sampler";
        default:                                return "Undefined";
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

Resource* ResourceBindingIterator::Next(BindingDescriptor& bindingDesc)
{
    while (iterator_ < count_)
    {
        /* Search for resource type of interest */
        auto resourceType = bindings_[iterator_].type;
        if (resourceType == typeOfInterest_ && (bindings_[iterator_].stageFlags & stagesOfInterest_) != 0)
        {
            /* Check for null pointer exception */
            if (auto resource = resourceViews_[iterator_].resource)
            {
                bindingDesc = bindings_[iterator_];
                ++iterator_;
                return resource;
            }
            ErrNullPointerResource(resourceType);
        }
        ++iterator_;
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
