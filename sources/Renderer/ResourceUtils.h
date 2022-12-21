/*
 * ResourceUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_UTILS_H
#define LLGL_RESOURCE_UTILS_H


#include <LLGL/ResourceFlags.h>
#include <LLGL/RenderSystemFlags.h>


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


} // /namespace LLGL


#endif



// ================================================================================
