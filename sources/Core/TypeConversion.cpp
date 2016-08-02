/*
 * TypeConversion.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TypeConversion.h"


namespace LLGL
{


LLGL_EXPORT unsigned int GetDataTypeSize(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Byte:
        case DataType::UByte:
            return 1;
        case DataType::Short:
        case DataType::UShort:
            return 2;
        case DataType::Float:
        case DataType::Int:
        case DataType::UInt:
            return 4;
        case DataType::Double:
            return 8;
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
