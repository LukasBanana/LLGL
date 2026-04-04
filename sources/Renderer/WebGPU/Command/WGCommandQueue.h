/*
 * WGCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_COMMAND_QUEUE_H
#define LLGL_WG_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        WGCommandQueue(WGPUDevice device);
        ~WGCommandQueue();

    private:

        WGPUQueue queue_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
