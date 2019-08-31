/*
 * CsImageFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsImageFlags.h"


namespace SharpLLGL
{


generic <typename T>
SrcImageDescriptor<T>::SrcImageDescriptor()
{
    Format      = ImageFormat::RGBA;
    DataType    = SharpLLGL::DataType::UInt8;
    Data        = nullptr;
}

generic <typename T>
SrcImageDescriptor<T>::SrcImageDescriptor(ImageFormat format, SharpLLGL::DataType dataType, array<T>^ data)
{
    Format      = format;
    DataType    = dataType;
    Data        = data;
}


} // /namespace SharpLLGL



// ================================================================================
