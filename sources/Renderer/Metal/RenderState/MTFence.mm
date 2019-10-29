/*
 * MTFence.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTFence.h"


namespace LLGL
{


MTFence::MTFence(id<MTLDevice> device) :
    native_ { [device newFence] }
{
}

MTFence::~MTFence()
{
    [native_ release];
}


} // /namespace LLGL



// ================================================================================
