/*
 * CsRenderTargetFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsRenderSystemChild.h"
#include "CsPipelineStateFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class AttachmentType
{
    Color,
    Depth,
    DepthStencil,
    Stencil,
};


/* ----- Structures ----- */

public ref class AttachmentDescriptor
{

    public:

        AttachmentDescriptor();
        AttachmentDescriptor(AttachmentType type);
        AttachmentDescriptor(AttachmentType type, Texture^ texture);
        AttachmentDescriptor(AttachmentType type, Texture^ texture, std::uint32_t mipLevel);
        AttachmentDescriptor(AttachmentType type, Texture^ texture, std::uint32_t mipLevel, std::uint32_t arrayLayer);

        property AttachmentType Type;
        property Texture^       Texture;
        property unsigned int   MipLevel;
        property unsigned int   ArrayLayer;

};

public ref class RenderTargetDescriptor
{

    public:

        RenderTargetDescriptor();

        property RenderPass^                    RenderPass;
        property Extent2D^                      Resolution;
        property unsigned int                   Samples;
        property bool                           CustomMultiSampling;
        property List<AttachmentDescriptor^>^   Attachments;

};


} // /namespace SharpLLGL



// ================================================================================
