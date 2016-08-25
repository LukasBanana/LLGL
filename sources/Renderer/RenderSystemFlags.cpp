/*
 * RenderSystemFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


LLGL_EXPORT std::size_t DataTypeSize(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Float:   return 4;
        case DataType::Double:  return 8;
        case DataType::Byte:    return 1;
        case DataType::UByte:   return 1;
        case DataType::Short:   return 2;
        case DataType::UShort:  return 2;
        case DataType::Int:     return 4;
        case DataType::UInt:    return 4;
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
