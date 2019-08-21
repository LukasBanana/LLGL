/*
 * CsTextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsTextureFlags.h"


namespace SharpLLGL
{


TextureDescriptor::TextureDescriptor()
{
    Type            = TextureType::Texture1D;
    BindFlags       = SharpLLGL::BindFlags::Sampled | SharpLLGL::BindFlags::ColorAttachment;
    CPUAccessFlags  = SharpLLGL::CPUAccessFlags::None;
    MiscFlags       = SharpLLGL::MiscFlags::FixedSamples;
    Format          = SharpLLGL::Format::RGBA8UNorm;
    Extent          = gcnew Extent3D(1, 1, 1);
    ArrayLayers     = 1;
    MipLevels       = 0;
    Samples         = 1;
}


} // /namespace SharpLLGL



// ================================================================================
