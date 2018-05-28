/*
 * BufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_FLAG_H
#define LLGL_BUFFER_FLAG_H


#include "Export.h"
#include "VertexFormat.h"
#include "IndexFormat.h"
#include "RenderSystemFlags.h"
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Hardware buffer type enumeration.
\see ResourceType
*/
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

//! Buffer flags enumeration.
struct BufferFlags
{
    enum
    {
        /**
        \brief Buffer mapping with CPU read access is required.
        \see RenderSystem::MapBuffer
        */
        MapReadAccess       = (1 << 0),

        /**
        \brief Buffer mapping with CPU write access is required.
        \see RenderSystem::MapBuffer
        */
        MapWriteAccess      = (1 << 1),

        /**
        \brief Hint to the renderer that the buffer will be frequently updated from the CPU.
        This is useful for a constant buffer for instance, that is updated by the host program every frame.
        \see RenderSystem::WriteBuffer
        */
        DynamicUsage        = (1 << 2),

        /*
        \brief Buffer mapping with CPU read and write access is required.
        \see BufferFlags::MapReadAccess
        \see BufferFlags::MapWriteAccess
        */
        MapReadWriteAccess  = (MapReadAccess | MapWriteAccess),
    };
};


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
        std::uint32_t       stride      = 0;
    };

    //! Hardware buffer type. By default BufferType::Vertex.
    BufferType      type            = BufferType::Vertex;

    /**
    \brief Buffer size (in bytes). This must not be larger than 'RenderingLimits::maxBufferSize'. By default 0.
    \remarks If the buffer type is a storage buffer (i.e. from the type BufferType::Storage), 'size' must be a multiple of 'storageBuffer.stride'.
    \see RenderingLimits::maxBufferSize
    */
    std::uint64_t   size            = 0;

    /**
    \brief Specifies the buffer creation flags. By default 0.
    \remarks This can be bitwise OR combination of the entries of the BufferFlags enumeration.
    \see BufferFlags
    */
    long            flags           = 0;

    //! Vertex buffer type descriptor appendix.
    VertexBuffer    vertexBuffer;

    //! Index buffer type descriptor appendix.
    IndexBuffer     indexBuffer;

    //! Storage buffer type descriptor appendix.
    StorageBuffer   storageBuffer;
};

#if 0//TODO: replace by ShaderReflection

/**
\brief Constant buffer shader-view descriptor structure.
\remarks This structure is used to describe the view of a constant buffer within a shader.
*/
struct ConstantBufferViewDescriptor
{
    //! Constant buffer name, i.e. the identifier used in the shader.
    std::string     name;

    //! Index of the constant buffer within the respective shader.
    std::uint32_t   index       = 0;

    //! Buffer size (in bytes).
    std::uint32_t   size        = 0;
};

/**
\brief Storage buffer shader-view descriptor structure.
\remarks This structure is used to describe the view of a storage buffer within a shader.
*/
struct StorageBufferViewDescriptor
{
    //! Storage buffer name, i.e. the identifier used in the shader.
    std::string         name;

    //! Index of the storage buffer within the respective shader.
    std::uint32_t       index   = 0;

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

#endif


} // /namespace LLGL


#endif



// ================================================================================
