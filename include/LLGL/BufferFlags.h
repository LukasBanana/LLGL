/*
 * BufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_FLAG_H
#define LLGL_BUFFER_FLAG_H


#include "Export.h"
#include "ResourceFlags.h"
#include "VertexFormat.h"
#include "IndexFormat.h"
#include "RenderSystemFlags.h"
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

#if 0 // TODO: replace this by <BindFlags> enum
/**
\brief Hardware buffer type enumeration.
\see ResourceType
\todo Maybe replace this enum by "ResourceType".
\todo Maybe replace this by "BindFlags" to support binding the same buffer for multiple purposes.
*/
enum class BufferType
{
    //! Vertex buffer type.
    Vertex,

    //! Index buffer type.
    Index,

    //! Constant buffer type (also called "Uniform Buffer Object").
    Constant,

    //! Storage buffer type (also called "Shader Storage Buffer Object" or "Read/Write Buffer").
    Storage,

    /**
    \brief Stream output buffer type (also called "Transform Feedback Buffer").
    \note Only supported with: OpenGL, Direct3D 11, Direct3D 12.
    */
    StreamOutput,

    #if 0//TODO: planned for v0.03
    /**
    \brief Result buffer type for query heaps.
    \see CommandBuffer::ResolveQuery
    */
    QueryResult,
    #endif
};
#endif // /TODO

/**
\brief Storage buffer type enumeration.
\note Only supported with: Direct3D 11, Direct3D 12.
*/
enum class StorageBufferType
{
    Undefined,                  //!< Undefined storage buffer type.
    Buffer,                     //!< Typed buffer.
    StructuredBuffer,           //!< Structured buffer.
    ByteAddressBuffer,          //!< Byte-address buffer.
    RWBuffer,                   //!< Typed read/write buffer.
    RWStructuredBuffer,         //!< Structured read/write buffer.
    RWByteAddressBuffer,        //!< Byte-address read/write buffer.
    AppendStructuredBuffer,     //!< Append structured buffer.
    ConsumeStructuredBuffer,    //!< Consume structured buffer.
};

#if 0 // TODO: replace this by <BindFlags> and <CPUAccessFlags>
/**
\brief Buffer creation flags enumeration.
\see BufferDescriptor::flags
*/
struct BufferFlags
{
    enum
    {
        /**
        \brief Buffer mapping with CPU read access is required.
        \see RenderSystem::MapBuffer
        */
        MapReadAccess           = (1 << 0),

        /**
        \brief Buffer mapping with CPU write access is required.
        \see RenderSystem::MapBuffer
        */
        MapWriteAccess          = (1 << 1),

        /*
        \brief Buffer mapping with CPU read and write access is required.
        \see BufferFlags::MapReadAccess
        \see BufferFlags::MapWriteAccess
        */
        MapReadWriteAccess      = (MapReadAccess | MapWriteAccess),

        /**
        \brief Hint to the renderer that the buffer will be frequently updated from the CPU.
        \remarks This is useful for a constant buffer for instance, that is updated by the host program every frame.
        \see RenderSystem::WriteBuffer
        */
        DynamicUsage            = (1 << 2),

        /**
        \brief Hint to the renderer that the buffer will hold the arguments for indirect commands.
        \remarks This must be used with a buffer of type BufferType::Storage.
        \see CommandBuffer::DrawIndirect
        \see CommandBuffer::DrawIndexedIndirect
        \see CommandBuffer::DispatchIndirect
        */
        IndirectBinding         = (1 << 8),
    };
};
#endif // /TODO


/* ----- Structures ----- */

//! Hardware buffer descriptor structure.
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
        \brief Specifies the index format layout, which is basically only the data type of each index.
        \remarks The only valid format types for an index buffer are: DataType::UByte, DataType::UShort, and DataType::UInt.
        \see DataType
        */
        IndexFormat format;
    };

    //! Storage buffer specific descriptor structure.
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
        \see IsStorageBufferTyped
        */
        Format              format      = Format::Undefined;

        /**
        \brief Specifies the stride (in bytes) of each element in a storage buffer.
        \remarks If this value is zero, the behavior of the buffer creation is undefined.
        */
        std::uint32_t       stride      = 0;
    };

    /**
    \brief Buffer size (in bytes). This must not be larger than 'RenderingLimits::maxBufferSize'. By default 0.
    \remarks If the buffer type is a storage buffer (i.e. from the type BufferType::Storage), 'size' must be a multiple of 'storageBuffer.stride'.
    \see RenderingLimits::maxBufferSize
    */
    std::uint64_t   size            = 0;

    #if 0 // TODO: replace this by "bindFlags" etc.
    //! Hardware buffer type. By default BufferType::Vertex.
    BufferType      type            = BufferType::Vertex;

    /**
    \brief Specifies the buffer creation flags. By default 0.
    \remarks This can be bitwise OR combination of the entries of the BufferFlags enumeration.
    \see BufferFlags
    */
    long            flags           = 0;
    #else
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
    #endif // /TODO

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
