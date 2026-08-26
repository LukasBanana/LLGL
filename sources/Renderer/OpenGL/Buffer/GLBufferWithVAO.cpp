/*
 * GLBufferWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(const BufferDescriptor& bufferDesc) :
    GLBuffer { bufferDesc }
{
}


} // /namespace LLGL



// ================================================================================
