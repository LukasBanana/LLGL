/*
 * GLTypeConversion.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTypeConversion.h"


namespace LLGL
{

namespace GLTypeConversion
{


GLenum Map(const BufferUsage bufferUsage)
{
    switch (bufferUsage)
    {
        case BufferUsage::Static:   return GL_STATIC_DRAW;
        case BufferUsage::Dynamic:  return GL_DYNAMIC_DRAW;
    }
    return 0;
}

GLenum Map(const BufferCPUAccess cpuAccess)
{
    switch (cpuAccess)
    {
        case BufferCPUAccess::ReadOnly:     return GL_READ_ONLY;
        case BufferCPUAccess::WriteOnly:    return GL_WRITE_ONLY;
        case BufferCPUAccess::ReadWrite:    return GL_READ_WRITE;
    }
    return 0;
}

GLenum Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Float:   return GL_FLOAT;
        case DataType::Double:  return GL_DOUBLE;
        case DataType::Byte:    return GL_BYTE;
        case DataType::UByte:   return GL_UNSIGNED_BYTE;
        case DataType::Short:   return GL_SHORT;
        case DataType::UShort:  return GL_UNSIGNED_SHORT;
        case DataType::Int:     return GL_INT;
        case DataType::UInt:    return GL_UNSIGNED_INT;
    }
    return 0;
}


} // /namespace GLTypeConversion

} // /namespace LLGL



// ================================================================================
