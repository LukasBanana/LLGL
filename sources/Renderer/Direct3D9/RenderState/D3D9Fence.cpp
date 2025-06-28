/*
 * D3D9Fence.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Fence.h"
#include <thread>
#include <chrono>


namespace LLGL
{


void D3D9Fence::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

void D3D9Fence::Signal(std::uint64_t signal)
{
    signal_ = signal;
}

void D3D9Fence::WaitForSignal(std::uint64_t signal)
{
    while (signal_ != signal)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

D3D9Fence::D3D9Fence(std::uint64_t initialSignal) :
    signal_ { initialSignal }
{
}


} // /namespace LLGL



// ================================================================================
