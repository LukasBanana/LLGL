/*
 * CsCommandBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsCommandBufferFlags.h"
#include "CsHelper.h"


namespace LHermanns
{

namespace SharpLLGL
{


/*
 * ClearValue class
 */

ClearValue::ClearValue()
{
    Color   = gcnew ColorRGBA();
    Depth   = 1.0f;
    Stencil = 0;
}


/*
 * AttachmentClear class
 */

AttachmentClear::AttachmentClear()
{
    Flags           = ClearFlags::None;
    ColorAttachment = 0;
    ClearValue      = gcnew LHermanns::SharpLLGL::ClearValue();
}

AttachmentClear::AttachmentClear(ColorRGBA^ color, unsigned int colorAttachment)
{
    Flags               = ClearFlags::Color;
    ColorAttachment     = colorAttachment;
    ClearValue          = gcnew LHermanns::SharpLLGL::ClearValue();
    ClearValue->Color   = color;
}

AttachmentClear::AttachmentClear(float depth)
{
    Flags               = ClearFlags::Depth;
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::SharpLLGL::ClearValue();
    ClearValue->Depth   = depth;
}

AttachmentClear::AttachmentClear(unsigned int stencil)
{
    Flags               = ClearFlags::Stencil;
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::SharpLLGL::ClearValue();
    ClearValue->Stencil = stencil;
}

AttachmentClear::AttachmentClear(float depth, unsigned int stencil)
{
    Flags               = ClearFlags::DepthStencil;
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::SharpLLGL::ClearValue();
    ClearValue->Depth   = depth;
    ClearValue->Stencil = stencil;
}


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
