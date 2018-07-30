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

namespace LLGL
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
    Flags           = 0;
    ColorAttachment = 0;
    ClearValue      = gcnew LHermanns::LLGL::ClearValue();
}

AttachmentClear::AttachmentClear(ColorRGBA^ color, unsigned int colorAttachment)
{
    Flags               = static_cast<int>(ClearFlags::Color);
    ColorAttachment     = colorAttachment;
    ClearValue          = gcnew LHermanns::LLGL::ClearValue();
    ClearValue->Color   = color;
}

AttachmentClear::AttachmentClear(float depth)
{
    Flags               = static_cast<int>(ClearFlags::Depth);
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::LLGL::ClearValue();
    ClearValue->Depth   = depth;
}

AttachmentClear::AttachmentClear(unsigned int stencil)
{
    Flags               = static_cast<int>(ClearFlags::Stencil);
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::LLGL::ClearValue();
    ClearValue->Stencil = stencil;
}

AttachmentClear::AttachmentClear(float depth, unsigned int stencil)
{
    Flags               = static_cast<int>(ClearFlags::DepthStencil);
    ColorAttachment     = 0;
    ClearValue          = gcnew LHermanns::LLGL::ClearValue();
    ClearValue->Depth   = depth;
    ClearValue->Stencil = stencil;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
