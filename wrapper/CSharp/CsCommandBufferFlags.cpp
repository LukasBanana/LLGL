/*
 * CsCommandBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsCommandBufferFlags.h"
#include "CsHelper.h"


namespace SharpLLGL
{


/*
 * ClearValue class
 */

ClearValue::ClearValue()
{
    Color   = gcnew ColorRGBA<float>(0.0f, 0.0f, 0.0f, 0.0f);
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
    ClearValue      = gcnew SharpLLGL::ClearValue();
}

AttachmentClear::AttachmentClear(ColorRGBA<float>^ color, unsigned int colorAttachment)
{
    Flags               = ClearFlags::Color;
    ColorAttachment     = colorAttachment;
    ClearValue          = gcnew SharpLLGL::ClearValue();
    ClearValue->Color   = color;
}

AttachmentClear::AttachmentClear(float depth)
{
    Flags               = ClearFlags::Depth;
    ColorAttachment     = 0;
    ClearValue          = gcnew SharpLLGL::ClearValue();
    ClearValue->Depth   = depth;
}

AttachmentClear::AttachmentClear(unsigned int stencil)
{
    Flags               = ClearFlags::Stencil;
    ColorAttachment     = 0;
    ClearValue          = gcnew SharpLLGL::ClearValue();
    ClearValue->Stencil = stencil;
}

AttachmentClear::AttachmentClear(float depth, unsigned int stencil)
{
    Flags               = ClearFlags::DepthStencil;
    ColorAttachment     = 0;
    ClearValue          = gcnew SharpLLGL::ClearValue();
    ClearValue->Depth   = depth;
    ClearValue->Stencil = stencil;
}


} // /namespace SharpLLGL



// ================================================================================
