/*
 * IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/IndexFormat.h>
#include "TypeConversion.h"


namespace LLGL
{


IndexFormat::IndexFormat(const DataType dataType) :
    formatSize_( GetDataTypeSize(dataType) )
{
}


} // /namespace LLGL



// ================================================================================
