/*
 * CsBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsBufferFlags.h"
#include <LLGL/Format.h>


namespace SharpLLGL
{


/*
 * BufferDescriptor class
 */

BufferDescriptor::BufferDescriptor()
{
    Size            = 0;
    BindFlags       = SharpLLGL::BindFlags::None;
    CPUAccessFlags  = SharpLLGL::CPUAccessFlags::None;
    MiscFlags       = SharpLLGL::MiscFlags::None;
    VertexAttribs   = gcnew List<VertexAttribute^>();
    IndexFormat     = Format::Undefined;
}


} // /namespace SharpLLGL



// ================================================================================
