/*
 * NullFence.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullFence.h"
#include <thread>
#include <chrono>


namespace LLGL
{


void NullFence::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

void NullFence::Signal(std::uint64_t signal)
{
    signal_ = signal;
}

void NullFence::WaitForSignal(std::uint64_t signal)
{
    while (signal_ != signal)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

NullFence::NullFence(std::uint64_t initialSignal) :
    signal_ { initialSignal }
{
}


} // /namespace LLGL



// ================================================================================
