/*
 * BindingDescriptorIterator.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BindingDescriptorIterator.h"
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>
#include "../Core/StringUtils.h"
#include "../Core/Exception.h"
#include <algorithm>


namespace LLGL
{


/*
 * BindingDescriptorIterator class
 */

BindingDescriptorIterator::BindingDescriptorIterator(const ArrayView<BindingDescriptor>& bindings) :
    bindings_ { bindings }
{
}

void BindingDescriptorIterator::Reset(const ResourceType typeOfInterest, long bindFlagsOfInterest, long stagesOfInterest)
{
    iterator_               = 0;
    typeOfInterest_         = typeOfInterest;
    bindFlagsOfInterest_    = bindFlagsOfInterest;
    stagesOfInterest_       = stagesOfInterest;
}

const BindingDescriptor* BindingDescriptorIterator::Next(std::size_t* outIndex)
{
    while (iterator_ < bindings_.size())
    {
        /* Search for resource type of interest */
        auto index = iterator_++;
        const auto& binding = bindings_[index % bindings_.size()];
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


/*
 * Global functions
 */

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
static void ErrNullPointerResource(ResourceType expectedType)
{
    LLGL_TRAP(
        "null pointer exception of resource object used as binding point for 'LLGL::ResourceType::%s'",
        ResourceTypeToString(expectedType)
    );
}

[[noreturn]]
static void ErrResourceTypeMismatch(ResourceType expectedType, ResourceType actualType)
{
    LLGL_TRAP(
        "type mismatch of resource object used as binding point: expected 'LLGL::ResourceType::%s', but got 'LLGL::ResourceType::%s'",
        ResourceTypeToString(expectedType), ResourceTypeToString(actualType)
    );
}

[[noreturn]]
static void ErrBindFlagsMismatch(ResourceType resourceType, long expectedBindFlags, long actualBindFlags)
{
    LLGL_TRAP(
        "binding flags mismatch of resource object (LLGL::ResourceType::%s) used as binding point: expected %s, but got %s",
        ResourceTypeToString(resourceType), IntToHex(expectedBindFlags), IntToHex(actualBindFlags)
    );
}

template <typename T, ResourceType ResType>
T* GetAsExpectedResource(Resource* resource)
{
    if (resource == nullptr)
        ErrNullPointerResource(ResType);
    if (resource->GetResourceType() != ResType)
        ErrResourceTypeMismatch(ResType, resource->GetResourceType());
    return static_cast<T*>(resource);
}

LLGL_EXPORT Buffer* GetAsExpectedBuffer(Resource* resource, long anyBindFlags)
{
    auto buffer = GetAsExpectedResource<Buffer, ResourceType::Buffer>(resource);
    if (anyBindFlags != 0 && (buffer->GetBindFlags() & anyBindFlags) == 0)
        ErrBindFlagsMismatch(ResourceType::Buffer, anyBindFlags, buffer->GetBindFlags());
    return buffer;
}

LLGL_EXPORT Texture* GetAsExpectedTexture(Resource* resource, long anyBindFlags)
{
    auto texture = GetAsExpectedResource<Texture, ResourceType::Texture>(resource);
    if (anyBindFlags != 0 && (texture->GetBindFlags() & anyBindFlags) == 0)
        ErrBindFlagsMismatch(ResourceType::Texture, anyBindFlags, texture->GetBindFlags());
    return texture;
}

LLGL_EXPORT Sampler* GetAsExpectedSampler(Resource* resource)
{
    return GetAsExpectedResource<Sampler, ResourceType::Sampler>(resource);
}


} // /namespace LLGL



// ================================================================================
