/*
 * CsCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/CommandQueue.h>
#include "CsCommandBuffer.h"
#include "CsRenderSystemChild.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class CommandQueue
{

    public:

        /* ----- Common ----- */

        CommandQueue(LLGL::CommandQueue* native);

        property LLGL::CommandQueue* Native
        {
            LLGL::CommandQueue* get();
        }

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer^ commandBuffer);

        #if 0
        /* ----- Queries ----- */

        bool QueryResult(
            QueryHeap^      queryHeap,
            System::UInt32  firstQuery,
            System::UInt32  numQueries,

        );
        #endif

        /* ----- Fences ----- */

        void Submit(Fence^ fence);

        bool WaitFence(Fence^ fence, UInt64 timeout);

        void WaitIdle();

    private:

        LLGL::CommandQueue* native_ = nullptr;

};


} // /namespace SharpLLGL



// ================================================================================
