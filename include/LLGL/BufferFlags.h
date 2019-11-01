/*
 * BufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_FLAG_H
#define LLGL_BUFFER_FLAG_H


#include "Export.h"
#include "ResourceFlags.h"
#include "VertexAttribute.h"
#include "RenderSystemFlags.h"
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Hardware buffer descriptor structure.
\see RenderSystem::CreateBuffer
*/
struct BufferDescriptor
{
    /**
    \brief Buffer size (in bytes). This must not be larger than RenderingLimits::maxBufferSize. By default 0.
    \remarks If \c stride is greater than zero, then \c size \b must be a multiple of \c stride.
    \see RenderingLimits::maxBufferSize
    \see bindFlags
    */
    std::uint64_t                   size            = 0;

    /**
    \brief Optional stride for structured buffers. By default 0.
    \remarks This is only used for Direct3D structured buffer, i.e. \c StructuredBuffer, \c RWStructuredBuffer, \c AppendStructuredBuffer, and \c ConsumeStructuredBuffer in HLSL.
    \remarks If this is non-zero, the \c format attribute is ignored for sampled and storage buffers, i.e. buffers with the binding flags BindFlags::Sampled or BindFlags::Storage.
    \note If the buffer has the binding flag BindFlags::IndirectBuffer, this \b must be 0.
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see MiscFlags::Append
    \see MiscFlags::Counter
    */
    std::uint32_t                   stride          = 0;

    /**
    \brief Optioanl hardware buffer format. By default Format::Undefined.
    \remarks This is used for index buffers, typed buffers (e.g. \c Buffer<uint4> and \c RWBuffer<float4> in HLSL), and byte address buffers (i.e. \c ByteAddressBuffer and \c RWByteAddressBuffer in HLSL).
    \remarks This field is ignored if the binding flags do not contain at least one of the following bits: BindFlags::IndexBuffer, BindFlags::Sampled, or BindFlags::Storage.
    \remarks If the BindFlags::IndexBuffer bit is set, this must be either Format::R16UInt, Format::R32UInt, or Format::Undefined.
    \remarks If Format::Undefined is specified and the BindFlags::IndexBuffer bit is set, only the secondary \c SetIndexBuffer function can be used in the CommandBuffer interface.
    \remarks If Format::Undefined is specified and \c stride is zero, sampled and storage buffers will be interpreted as byte address buffers.
    \see BindFlags::IndexBuffer
    \see CommandBuffer::SetIndexBuffer(Buffer&)
    */
    Format                          format          = Format::Undefined;

    /**
    \brief These flags describe to which resource slots the buffer can be bound. By default 0.
    \remarks When the buffer will be bound to a vertex buffer slot for instance, the BindFlags::VertexBuffer flag is required.
    \see BindFlags
    */
    long                            bindFlags       = 0;

    /**
    \brief CPU read/write access flags. By default 0.
    \remarks If this is 0 the buffer cannot be mapped from GPU memory space into CPU memory space and vice versa.
    \see CPUAccessFlags
    \see RenderSystem::MapBuffer
    */
    long                            cpuAccessFlags  = 0;

    /**
    \brief Miscellaneous buffer flags. By default 0.
    \remarks This can be used as a hint for the renderer how frequently the buffer will be updated.
    \see MiscFlags
    */
    long                            miscFlags       = 0;

    /**
    \brief Specifies the list of vertex attributes.
    \remarks This is only used for vertex buffers and ignored if \c bindFlags does not contain the BindFlags::VertexBuffer bit.
    \see BindFlags::VertexBuffer
    \see VertexShaderAttributes::inputAttribs
    */
    std::vector<VertexAttribute>    vertexAttribs;
};


/* ----- Functions ----- */

/**
\defgroup group_buffer_util Buffer utility functions to determine buffer types.
\addtogroup group_buffer_util
@{
*/

//! Returns true if the buffer descriptor denotes a typed buffer, i.e. \c Buffer or \c RWBuffer in HLSL.
LLGL_EXPORT bool IsTypedBuffer(const BufferDescriptor& desc);

/**
\brief Returns true if the buffer descriptor denotes a structured buffer,
i.e. \c StructuredBuffer, \c RWStructuredBuffer, \c AppendStructuredBuffer, or \c ConsumeStructuredBuffer in HLSL.
*/
LLGL_EXPORT bool IsStructuredBuffer(const BufferDescriptor& desc);

/**
\brief Returns true if the buffer descriptor denotes a byte addresse buffer, i.e. \c ByteAddressBuffer or \c RWByteAddressBuffer in HLSL.
*/
LLGL_EXPORT bool IsByteAddressBuffer(const BufferDescriptor& desc);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
