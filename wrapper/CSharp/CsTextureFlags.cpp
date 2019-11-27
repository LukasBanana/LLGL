/*
 * CsTextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsTextureFlags.h"


namespace SharpLLGL
{


TextureDescriptor::TextureDescriptor()
{
    Type            = TextureType::Texture2D;
    BindFlags       = SharpLLGL::BindFlags::Sampled | SharpLLGL::BindFlags::ColorAttachment;
    MiscFlags       = SharpLLGL::MiscFlags::FixedSamples | SharpLLGL::MiscFlags::GenerateMips;
    Format          = SharpLLGL::Format::RGBA8UNorm;
    Extent          = gcnew Extent3D(1, 1, 1);
    ArrayLayers     = 1;
    MipLevels       = 0;
    Samples         = 1;
}


} // /namespace SharpLLGL



// ================================================================================
