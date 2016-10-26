/*
 * BufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_FLAG_H
#define LLGL_BUFFER_FLAG_H


#include "Export.h"
#include "VertexFormat.h"
#include "IndexFormat.h"
#include "RenderSystemFlags.h"
#include <string>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Hardware buffer type enumeration.
enum class BufferType
{
    Vertex,         //!< Vertex buffer type.
    Index,          //!< Index buffer type.
    Constant,       //!< Constant buffer type (also called "Uniform Buffer Object").
    Storage,        //!< Storage buffer type (also called "Shader Storage Buffer Object" or "Read/Write Buffer").
    StreamOutput,   //!< Stream output buffer type (also called "Transform Feedback Buffer").
};

/**
\brief Storage buffer type enumeration.
\note Only supported with: Direct3D 11, Direct3D 12.
*/
enum class StorageBufferType
{
    Buffer,                     //!< Typed buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    StructuredBuffer,           //!< Structured buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    ByteAddressBuffer,          //!< Byte-address buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    RWBuffer,                   //!< Typed read/write buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    RWStructuredBuffer,         //!< Structured read/write buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    RWByteAddressBuffer,        //!< Byte-address read/write buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    AppendStructuredBuffer,     //!< Append structured buffer. \note Only supported with: Direct3D 11, Direct3D 12.
    ConsumeStructuredBuffer,    //!< Consume structured buffer. \note Only supported with: Direct3D 11, Direct3D 12.
};

/**
\brief Hardware buffer CPU acccess enumeration.
\see RenderSystem::MapBuffer
*/
enum class BufferCPUAccess
{
    ReadOnly,   //!< CPU read access only.
    WriteOnly,  //!< CPU write access only.
    ReadWrite,  //!< CPU read and write access.
};

//! Buffer flags enumeration.
struct BufferFlags
{
    enum
    {
        /**
        \brief Buffer mapping with CPU read access is required.
        \see RenderSystem::MapBuffer
        */
        MapReadAccess   = (1 << 0),

        /**
        \brief Buffer mapping with CPU write access is required.
        \see RenderSystem::MapBuffer
        */
        MapWriteAccess  = (1 << 1),

        /**
        \brief Hint to the renderer that the buffer will be frequently updated from the CPU.
        This is useful for a constant buffer for instance, that is updated by the host program every frame.
        \see RenderSystem::WriteBuffer
        */
        DynamicUsage    = (1 << 2),
    };
};


/* ----- Structures ----- */

//! Hardware buffer descriptor structure.
struct BufferDescriptor
{
    //! Vertex buffer descriptor structure.
    struct VertexBufferDescriptor
    {
        /**
        \brief Specifies the vertex format layout.
        \remarks This is required to tell the renderer how the vertex attributes are stored inside the vertex buffer and
        it must be the same vertex format which is used for the respective graphics pipeline shader program.
        */
        VertexFormat format;
    };

    struct IndexBufferDescriptor
    {
        /**
        \brief Specifies the index format layout, which is basically only the data type of each index.
        \remarks The only valid format types for an index buffer are: DataType::UByte, DataType::UShort, and DataType::UInt.
        \see DataType
        */
        IndexFormat format;
    };

    struct StorageBufferDescriptor
    {
        /**
        \brief Specifies the storage buffer type. By defalut StorageBufferType::Buffer.
        \remarks In OpenGL there are only generic storage buffers (or rather "Shader Storage Buffer Objects").
        \see vectorType
        */
        StorageBufferType   storageType = StorageBufferType::Buffer;

        /**
        \brief Specifies the vector type of a typed buffer.
        \remarks This is only used if the storage type is either StorageBufferType::Buffer or StorageBufferType::RWBuffer.
        */
        VectorType          vectorType  = VectorType::Float4;

        /**
        \brief Specifies the stride (in bytes) of each element in a storage buffer.
        \remarks If this value is zero, the behavior of the buffer creation is undefined.
        */
        unsigned int        stride      = 0;
    };

    //! Hardware buffer type. By default BufferType::Vertex.
    BufferType              type    = BufferType::Vertex;

    /**
    \brief Buffer size (in bytes). By default 0.
    \remarks If the buffer type is a storage buffer (i.e. from the type BufferType::Storage),
    'size' must be a multiple of 'storageBuffer.stride'.
    */
    unsigned int            size    = 0;

    /**
    \brief Specifies the buffer creation flags. By default 0.
    \remarks This can be bitwise OR combination of the entries of the BufferFlags enumeration.
    \see BufferFlags
    */
    long                    flags   = 0;

    //! Vertex buffer type descriptor appendix.
    VertexBufferDescriptor  vertexBuffer;

    //! Index buffer type descriptor appendix.
    IndexBufferDescriptor   indexBuffer;

    //! Storage buffer type descriptor appendix.
    StorageBufferDescriptor storageBuffer;
};

/**
\brief Constant buffer shader-view descriptor structure.
\remarks This structure is used to describe the view of a constant buffer within a shader.
*/
struct ConstantBufferViewDescriptor
{
    std::string     name;           //!< Constant buffer name.
    unsigned int    index   = 0;    //!< Index of the constant buffer within the respective shader.
    unsigned int    size    = 0;    //!< Buffer size (in bytes).
};

/**
\brief Storage buffer shader-view descriptor structure.
\remarks This structure is used to describe the view of a storage buffer within a shader.
*/
struct StorageBufferViewDescriptor
{
    std::string         name;           //!< Storage buffer name.
    unsigned int        index   = 0;    //!< Index of the storage buffer within the respective shader.

    /**
    \brief Storage buffer type.
    \remarks For the OpenGL render system, this type is always 'StorageBufferType::Buffer',
    since GLSL only supports generic shader storage buffers. Here is an example:
    \code
    layout(std430, binding=0) buffer myBuffer
    {
        vec4 myBufferArray[];
    };
    \endcode
    \note Only supported with: Direct3D 11, Direct3D 12
    */
    StorageBufferType   type    = StorageBufferType::Buffer;
};


} // /namespace LLGL


#endif



// ================================================================================
