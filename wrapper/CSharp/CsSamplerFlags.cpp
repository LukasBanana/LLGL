/*
 * CsSamplerFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsSamplerFlags.h"


namespace SharpLLGL
{


SamplerDescriptor::SamplerDescriptor()
{
    AddressModeU    = SamplerAddressMode::Repeat;
    AddressModeV    = SamplerAddressMode::Repeat;
    AddressModeW    = SamplerAddressMode::Repeat;
    MinFilter       = SamplerFilter::Linear;
    MagFilter       = SamplerFilter::Linear;
    MipMapFilter    = SamplerFilter::Linear;
    MipMapping      = true;
    MipMapLODBias   = 0.0f;
    MinLOD          = 0.0f;
    MaxLOD          = 1000.0f;
    MaxAnisotropy   = 1;
    CompareEnabled  = false;
    CompareOp       = SharpLLGL::CompareOp::Less;
    BorderColor     = gcnew ColorRGBA<float>(0.0f, 0.0f, 0.0f, 0.0f);
}


} // /namespace SharpLLGL



// ================================================================================
