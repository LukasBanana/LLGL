/*
 * CsCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/CommandQueue.h>
#include "CsCommandBuffer.h"
#include "CsRenderSystemChilds.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LHermanns
{

namespace LLGL
{


public ref class CommandQueue
{

    public:

        /* ----- Common ----- */

        CommandQueue(::LLGL::CommandQueue* native);

        property ::LLGL::CommandQueue* Native
        {
            ::LLGL::CommandQueue* get();
        }

        /* ----- Command Buffers ----- */

        void Begin(CommandBuffer^ commandBuffer);
        void Begin(CommandBuffer^ commandBuffer, int flags);

        void End(CommandBuffer^ commandBuffer);

        void Submit(CommandBuffer^ commandBuffer);

        /* ----- Fences ----- */

        void Submit(Fence^ fence);

        bool WaitFence(Fence^ fence, UInt64 timeout);

        void WaitIdle();

    private:

        ::LLGL::CommandQueue* native_ = nullptr;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
