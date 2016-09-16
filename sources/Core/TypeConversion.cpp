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
        case DataType::Int8:
        case DataType::UInt8:
            return 1;
        case DataType::Int16:
        case DataType::UInt16:
            return 2;
        case DataType::Float32:
        case DataType::Int32:
        case DataType::UInt32:
            return 4;
        case DataType::Float64:
            return 8;
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
