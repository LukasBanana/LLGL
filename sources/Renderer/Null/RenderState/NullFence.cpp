/*
 * NullFence.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullFence.h"
#include <thread>
#include <chrono>


namespace LLGL
{


void NullFence::SetName(const char* name)
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


} // /namespace LLGL



// ================================================================================
