/*
 * CsRenderTargetFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderTargetFlags.h"


namespace SharpLLGL
{


/*
 * AttachmentDescriptor class
 */

AttachmentDescriptor::AttachmentDescriptor()
{
    Type        = AttachmentType::Color;
    Texture     = nullptr;
    MipLevel    = 0;
    ArrayLayer  = 0;
}

AttachmentDescriptor::AttachmentDescriptor(AttachmentType type)
{
    Type        = type;
    Texture     = nullptr;
    MipLevel    = 0;
    ArrayLayer  = 0;
}

AttachmentDescriptor::AttachmentDescriptor(AttachmentType type, SharpLLGL::Texture^ texture)
{
    Type        = type;
    Texture     = texture;
    MipLevel    = 0;
    ArrayLayer  = 0;
}

AttachmentDescriptor::AttachmentDescriptor(AttachmentType type, SharpLLGL::Texture^ texture, std::uint32_t mipLevel)
{
    Type        = type;
    Texture     = texture;
    MipLevel    = mipLevel;
    ArrayLayer  = 0;
}

AttachmentDescriptor::AttachmentDescriptor(AttachmentType type, SharpLLGL::Texture^ texture, std::uint32_t mipLevel, std::uint32_t arrayLayer)
{
    Type        = type;
    Texture     = texture;
    MipLevel    = mipLevel;
    ArrayLayer  = arrayLayer;
}


/*
 * RenderTargetDescriptor class
 */

RenderTargetDescriptor::RenderTargetDescriptor()
{
    RenderPass          = nullptr;
    Resolution          = gcnew Extent2D();
    Samples             = 1;
    CustomMultiSampling = false;
    Attachments         = gcnew List<AttachmentDescriptor^>();
}


} // /namespace SharpLLGL



// ================================================================================
