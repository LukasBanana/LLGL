/*
 * Format.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Format.h>


namespace LLGL
{


LLGL_EXPORT unsigned int DataTypeSize(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Int8:
        case DataType::UInt8:
            return 1;
        case DataType::Int16:
        case DataType::UInt16:
            return 2;
        case DataType::Int32:
        case DataType::UInt32:
        case DataType::Float:
            return 4;
        case DataType::Double:
            return 8;
    }
    return 0;
}

LLGL_EXPORT unsigned int VectorTypeSize(const VectorType vectorType)
{
    DataType        dataType    = DataType::Float;
    unsigned int    components  = 0;
    VectorTypeFormat(vectorType, dataType, components);

    return (DataTypeSize(dataType) * components);
}

LLGL_EXPORT void VectorTypeFormat(const VectorType vectorType, DataType& dataType, unsigned int& components)
{
    /* Get data type and components by indexed vector type */
    auto vectorTypeIdx = (static_cast<int>(vectorType) - static_cast<int>(VectorType::Float));
    auto componentsIdx = vectorTypeIdx % 4;
    vectorTypeIdx /= 4;

    if (vectorTypeIdx < 4)
    {
        static const DataType vecDataTypes[] = { DataType::Float, DataType::Double, DataType::Int32, DataType::UInt32 };
        dataType    = vecDataTypes[vectorTypeIdx];
        components  = (componentsIdx + 1);
    }
}


} // /namespace LLGL



// ================================================================================
