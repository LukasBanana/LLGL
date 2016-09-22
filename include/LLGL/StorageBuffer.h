/*
 * StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_STORAGE_BUFFER_H__
#define __LLGL_STORAGE_BUFFER_H__


#include "Export.h"
#include <string>


namespace LLGL
{


/**
\brief Storage buffer type enumeration.
\note Only supported with: Direct3D 11, Direct3D 12.
*/
enum class StorageBufferType
{
    Buffer,                     //!< Typed buffer.
    RWBuffer,                   //!< Typed read/write buffer.
    StructuredBuffer,           //!< Structured buffer.
    RWStructuredBuffer,         //!< Structured read/write buffer.
    ByteAddressBuffer,          //!< Byte-address buffer.
    RWByteAddressBuffer,        //!< Byte-address read/write buffer.
    AppendStructuredBuffer,     //!< Append structured buffer.
    ConsumeStructuredBuffer,    //!< Consume structured buffer.
};

//! Storage buffer descriptor structure.
struct StorageBufferDescriptor
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

//! Storage buffer (also called "Shader Storage Buffer Object" or "Read/Write Buffer") interface.
class LLGL_EXPORT StorageBuffer
{

    public:

        virtual ~StorageBuffer()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
