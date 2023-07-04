/*
 * NativeHandle.h (Metal)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_METAL_NATIVE_HANDLE_H
#define LLGL_METAL_NATIVE_HANDLE_H


#import <Metal/Metal.h>


namespace LLGL
{

namespace Metal
{


struct RenderSystemNativeHandle
{
    id<MTLDevice> device;
};

struct CommandBufferNativeHandle
{
    id<MTLCommandBuffer> commandBuffer;
};


} // /namespace Metal

} // /namespace LLGL


#endif



// ================================================================================
