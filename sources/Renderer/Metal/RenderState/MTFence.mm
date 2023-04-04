/*
 * MTFence.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
