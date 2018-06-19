/*
 * Format.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Format.h>


namespace LLGL
{


LLGL_EXPORT bool IsCompressedFormat(const Format format)
{
    return (format >= Format::BC1RGB);
}

LLGL_EXPORT bool IsDepthStencilFormat(const Format format)
{
    return (format == Format::D32Float || format == Format::D24UNormS8UInt);
}

LLGL_EXPORT std::uint32_t DataTypeSize(const DataType dataType)
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

LLGL_EXPORT bool IsIntDataType(const DataType dataType)
{
    return (dataType == DataType::Int8 || dataType == DataType::Int16 || dataType == DataType::Int32);
}

LLGL_EXPORT bool IsUIntDataType(const DataType dataType)
{
    return (dataType == DataType::UInt8 || dataType == DataType::UInt16 || dataType == DataType::UInt32);
}

LLGL_EXPORT bool IsFloatDataType(const DataType dataType)
{
    return (dataType == DataType::Float || dataType == DataType::Double);
}

LLGL_EXPORT std::uint32_t VectorTypeSize(const VectorType vectorType)
{
    DataType dataType   = DataType::Float;
    std::uint32_t components = 0;
    VectorTypeFormat(vectorType, dataType, components);

    return (DataTypeSize(dataType) * components);
}

LLGL_EXPORT void VectorTypeFormat(const VectorType vectorType, DataType& dataType, std::uint32_t& components)
{
    /* Get data type and components by indexed vector type */
    auto vectorTypeIdx = (static_cast<std::uint32_t>(vectorType) - static_cast<std::uint32_t>(VectorType::Float));
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
