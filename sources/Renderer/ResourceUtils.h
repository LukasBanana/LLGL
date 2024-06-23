/*
 * ResourceUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RESOURCE_UTILS_H
#define LLGL_RESOURCE_UTILS_H


#include <LLGL/Export.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/DynamicVector.h>
#include <string>


namespace LLGL
{


/* ----- Enumerations ----- */

// Enumeration of predefined static sampler border colors.
enum class StaticSamplerBorderColor
{
    TransparentBlack,   // Predefined border color { 0, 0, 0, 0 }
    OpaqueBlack,        // Predefined border color { 1, 1, 1, 0 }
    OpaqueWhite,        // Predefined border color { 1, 1, 1, 1 }
};


/* ----- Functions ----- */

// Returns true if the specified flags contain any input binding flags.
inline bool HasInputBindFlags(long bindFlags)
{
    constexpr long inputBindFlags = (BindFlags::Sampled | BindFlags::CopySrc | BindFlags::VertexBuffer | BindFlags::IndexBuffer | BindFlags::ConstantBuffer | BindFlags::IndirectBuffer);
    return ((bindFlags & inputBindFlags) != 0);
}

// Returns true if the specified flags contain any output binding flags.
inline bool HasOutputBindFlags(long bindFlags)
{
    constexpr long outputBindFlags = (BindFlags::Storage | BindFlags::CopyDst | BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment | BindFlags::StreamOutputBuffer);
    return ((bindFlags & outputBindFlags) != 0);
}

// Returns true if the specified CPU access value has read access, i.e. ReadOnly or ReadWrite.
inline bool HasReadAccess(const CPUAccess access)
{
    return (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite);
}

// Returns true if the specified CPU access value has write access, i.e. WriteOnly, WriteDiscard, or ReadWrite.
inline bool HasWriteAccess(const CPUAccess access)
{
    return (access >= CPUAccess::WriteOnly && access <= CPUAccess::ReadWrite);
}

// Returns the number of resource views for the specified resource heap descriptor and throws an std::invalid_argument exception if validation fails.
LLGL_EXPORT std::uint32_t GetNumResourceViewsOrThrow(
    std::uint32_t                               numBindings,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews
);

// Returns the enumeration value for a predefined static sampler border color.
LLGL_EXPORT StaticSamplerBorderColor GetStaticSamplerBorderColor(const float (&color)[4]);

// Returns a list of expanded heap-binding descriptors, i.e. all array resources have been flattened.
LLGL_EXPORT DynamicVector<BindingDescriptor> GetExpandedHeapDescriptors(const ArrayView<BindingDescriptor>& bindingDescs);


} // /namespace LLGL


#endif



// ================================================================================
