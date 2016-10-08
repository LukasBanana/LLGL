/*
 * BufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_BUFFER_FLAG_H__
#define __LLGL_BUFFER_FLAG_H__


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
    /* ----- OpenGL's Shader Storage Buffer Object (SSBO) types ----- */
    Generic,                    //!< Generic storage buffer type. \note Only supported with: OpenGL.

    /* ----- Direct3D's Read/Write Buffer types ----- */
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
\brief Hardware buffer usage enumeration.
\remarks For OpenGL, the buffer usage is just a hint to the GL server.
For Direct3D, the buffer usage is crucial during buffer creation.
\see RenderSystem::CreateBuffer
*/
enum class BufferUsage
{
    /**
    \brief The hardware buffer will be rarely changed by the client but often used by the hardware.
    \remarks For Direct3D 11, a buffer can use the static buffer usage, if always the entire buffer will be updated.
    Otherwise, the dynamic buffer usage must be used.
    */
    Static,

    /**
    \brief The hardware buffer will be often changed by the client (e.g. almost every frame).
    \remarks For Direct3D 11, a buffer must use the dynamic buffer usage, if it will only partially be updated at any time.
    */
    Dynamic,
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
        VertexFormat vertexFormat;
    };

    struct IndexBufferDescriptor
    {
        /**
        \brief Specifies the index format layout, which is basically only the data type of each index.
        \remarks The only valid format types for an index buffer are: DataType::UByte, DataType::UShort, and DataType::UInt.
        \see DataType
        */
        IndexFormat indexFormat;
    };

    struct StorageBufferDescriptor
    {
        /**
        \brief Specifies the storage buffer type.
        \remarks In OpenGL there are only generic storage buffers (or rather "Shader Storage Buffer Objects").
        */
        StorageBufferType   storageType = StorageBufferType::Generic;

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

    //! Buffer usage. By default BufferUsage::Static.
    BufferUsage             usage   = BufferUsage::Static;

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
