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
#include "VertexFormat.h"
#include "RenderSystemFlags.h"
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Storage buffer type enumeration.
\note Only supported with: Direct3D 11, Direct3D 12.
\see BufferDescriptor::StorageBuffer::storageType
\todo Replace by BindFlag with different options (e.g. StructuredBuffer and Storage).
*/
enum class StorageBufferType
{
    //! Undefined storage buffer type.
    Undefined,

    //! Typed buffer.
    Buffer,

    /**
    \brief Structured buffer.
    \note This cannot be used together with a buffer that has the binding flag BindFlags::IndirectBuffer.
    */
    StructuredBuffer,

    //! Byte-address buffer.
    ByteAddressBuffer,

    //! Typed read/write buffer.
    RWBuffer,

    /**
    \brief Structured read/write buffer.
    \note This cannot be used together with a buffer that has the binding flag BindFlags::IndirectBuffer.
    */
    RWStructuredBuffer,

    //! Byte-address read/write buffer.
    RWByteAddressBuffer,

    /**
    \brief Append structured buffer.
    \note This cannot be used together with a buffer that has the binding flag BindFlags::IndirectBuffer.
    */
    AppendStructuredBuffer,

    /**
    \brief Consume structured buffer.
    \note This cannot be used together with a buffer that has the binding flag BindFlags::IndirectBuffer.
    */
    ConsumeStructuredBuffer,
};


/* ----- Structures ----- */

/**
\brief Hardware buffer descriptor structure.
\see RenderSystem::CreateBuffer
*/
struct BufferDescriptor
{
    //! Vertex buffer specific descriptor structure.
    struct VertexBuffer
    {
        /**
        \brief Specifies the vertex format layout.
        \remarks This is required to tell the renderer how the vertex attributes are stored inside the vertex buffer and
        it must be the same vertex format which is used for the respective graphics pipeline shader program.
        */
        VertexFormat format;
    };

    //! Index buffer specific descriptor structure.
    struct IndexBuffer
    {
        /**
        \brief Specifies the format of each index in the buffer. This must be either Format::R16UInt or Format::R32UInt. By default Format::R32UInt.
        \see Format
        */
        Format format = Format::R32UInt;
    };

    /**
    \brief Storage buffer specific descriptor structure.
    \todo Move this into a BufferViewDescriptor.
    */
    struct StorageBuffer
    {
        /**
        \brief Specifies the storage buffer type. By defalut StorageBufferType::Undefined.
        \remarks In OpenGL there are only generic storage buffers (or rather "Shader Storage Buffer Objects").
        However, a valid type should always be specified when a storage buffer is created.
        */
        StorageBufferType   storageType = StorageBufferType::Undefined;

        /**
        \brief Specifies the vector format of a typed buffer. By default Format::Undefined.
        \remarks This is only used if the storage type is either StorageBufferType::Buffer or StorageBufferType::RWBuffer.
        \see IsTypedBuffer
        */
        Format              format      = Format::Undefined;

        /**
        \brief Specifies the stride (in bytes) of each element in a storage buffer. By default 0.
        \remarks If the buffer has the BindFlags::Sampled or BindFlags::Storage flag, then \c stride must not be zero.
        \see BufferDescriptor::bindFlags
        */
        std::uint32_t       stride      = 0;
    };

    /**
    \brief Buffer size (in bytes). This must not be larger than RenderingLimits::maxBufferSize. By default 0.
    \remarks If the buffer has the BindFlags::Sampled or BindFlags::Storage flag, then \c size must be a multiple of <code>storageBuffer.stride</code>.
    \see RenderingLimits::maxBufferSize
    \see bindFlags
    \see StorageBuffer::stride
    */
    std::uint64_t   size            = 0;

    /**
    \brief These flags describe to which resource slots the buffer can be bound. By default 0.
    \remarks When the buffer will be bound to a vertex buffer slot for instance, the BindFlags::VertexBuffer flag is required.
    \see BindFlags
    */
    long            bindFlags       = 0;

    /**
    \brief CPU read/write access flags. By default 0.
    \remarks If this is 0 the buffer cannot be mapped from GPU memory space into CPU memory space and vice versa.
    \see CPUAccessFlags
    \see RenderSystem::MapBuffer
    */
    long            cpuAccessFlags  = 0;

    /**
    \brief Miscellaneous buffer flags. By default 0.
    \remarks This can be used as a hint for the renderer how frequently the buffer will be updated.
    \see MiscFlags
    */
    long            miscFlags       = 0;

    //! Vertex buffer type descriptor appendix.
    VertexBuffer    vertexBuffer;

    //! Index buffer type descriptor appendix.
    IndexBuffer     indexBuffer;

    //! Storage buffer type descriptor appendix.
    StorageBuffer   storageBuffer;
};


/* ----- Functions ----- */

/**
\defgroup group_buffer_util Buffer utility functions to determine buffer types.
\addtogroup group_buffer_util
@{
*/

/**
\brief Returns true if the storage buffer type denotes a read/write (RW) buffer.
\return True if \c type either StorageBufferType::RWBuffer, StorageBufferType::RWStructuredBuffer,
StorageBufferType::RWByteAddressBuffer, StorageBufferType::AppendStructuredBuffer, or StorageBufferType::ConsumeStructuredBuffer.
*/
LLGL_EXPORT bool IsRWBuffer(const StorageBufferType type);

/**
\brief Returns true if the storage buffer type denotes a simply typed buffer.
\return True if \c type either StorageBufferType::Buffer or StorageBufferType::RWBuffer.
*/
LLGL_EXPORT bool IsTypedBuffer(const StorageBufferType type);

/**
\brief Returns true if the storage buffer type denotes a structured buffer.
\return True if \c type either StorageBufferType::StructuredBuffer, StorageBufferType::RWStructuredBuffer,
StorageBufferType::AppendStructuredBuffer, or StorageBufferType::ConsumeStructuredBuffer.
*/
LLGL_EXPORT bool IsStructuredBuffer(const StorageBufferType type);

/**
\brief Returns true if the storage buffer type denotes a byte addresse buffer.
\return True if \c type either StorageBufferType::ByteAddressBuffer or StorageBufferType::RWByteAddressBuffer.
*/
LLGL_EXPORT bool IsByteAddressBuffer(const StorageBufferType type);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
