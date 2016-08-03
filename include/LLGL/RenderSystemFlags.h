/*
 * RenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_FLAGS_H__
#define __LLGL_RENDER_SYSTEM_FLAGS_H__


namespace LLGL
{


/**
\brief Hardware buffer usage enumeration.
\see RenderSystem::WriteVertexBuffer
\see RenderSystem::WriteIndexBuffer
*/
enum class BufferUsage
{
    Static,     //!< The hardware buffer will change rarely by the client but often used by the hardware.
    Dynamic,    //!< The hardware buffer will change often by the client.
};

/**
Hardware buffer CPU acccess enumeration.
\see RenderSystem::MapBuffer
*/
enum class BufferCPUAccess
{
    ReadOnly,   //!< CPU read access only.
    WriteOnly,  //!< CPU write access only.
    ReadWrite,  //!< CPU read and write access.
};

//! Renderer data types enumeration.
enum class DataType
{
    Float,  //!< 32-bit floating-point.
    Double, //!< 64-bit floating-point.
    
    Byte,   //!< 8-bit signed integer.
    UByte,  //!< 8-bit unsigned integer.

    Short,  //!< 16-bit signed integer.
    UShort, //!< 16-bit unsigned integer.

    Int,    //!< 32-bit signed integer.
    UInt,   //!< 32-bit unsigned integer.
};


} // /namespace LLGL


#endif



// ================================================================================
