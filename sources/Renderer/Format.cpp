/*
 * Format.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Format.h>


namespace LLGL
{


LLGL_EXPORT void VectorTypeFormat(const VectorType vectorType, DataType& dataType, unsigned int& components)
{
    /* Get data type and components by indexed vector type */
    auto dataTypeIdx    = (static_cast<int>(vectorType) - static_cast<int>(VectorType::Float));
    auto componentsIdx  = dataTypeIdx % 4;
    dataTypeIdx /= 4;

    if (dataTypeIdx < 4)
    {
        static const DataType vecDataTypes[] = { DataType::Float, DataType::Double, DataType::Int32, DataType::UInt32 };
        dataType    = vecDataTypes[dataTypeIdx];
        components  = (componentsIdx + 1);
    }
}


} // /namespace LLGL



// ================================================================================
