/*
 * ResourceUtils.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_UTILS_H
#define LLGL_RESOURCE_UTILS_H


#include <LLGL/ResourceFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <stdexcept>
#include <string>


namespace LLGL
{


/* ----- Functions ----- */

// Returns true if the specified flags contain any input binding flags.
inline bool HasInputBindFlags(long bindFlags)
{
    const long inputBindFlags = (BindFlags::Sampled | BindFlags::CopySrc | BindFlags::VertexBuffer | BindFlags::IndexBuffer | BindFlags::ConstantBuffer | BindFlags::IndirectBuffer);
    return ((bindFlags & inputBindFlags) != 0);
}

// Returns true if the specified flags contain any output binding flags.
inline bool HasOutputBindFlags(long bindFlags)
{
    const long outputBindFlags = (BindFlags::Storage | BindFlags::CopyDst | BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment | BindFlags::StreamOutputBuffer);
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
inline std::uint32_t GetNumResourceViewsOrThrow(
    std::uint32_t                               numBindings,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Resource heaps cannot have pipeline layout with no bindings */
    if (numBindings == 0)
        throw std::invalid_argument("cannot create resource heap without bindings in pipeline layout");

    /* Resource heaps cannot be empty */
    const std::uint32_t numResourceViews = (desc.numResourceViews > 0 ? desc.numResourceViews : static_cast<std::uint32_t>(initialResourceViews.size()));
    if (numResourceViews == 0)
        throw std::invalid_argument("cannot create empty resource heap");

    /* Number of resources must be a multiple of bindings */
    if (numResourceViews % numBindings != 0)
    {
        throw std::invalid_argument(
            "cannot create resource heap due because number of resources (" + std::to_string(numResourceViews) +
            ") is not a multiple of bindings (" + std::to_string(numBindings) + ")"
        );
    }

    return numResourceViews;
}


} // /namespace LLGL


#endif



// ================================================================================
