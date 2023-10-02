/*
 * Barrier.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Barrier.h"


namespace LLGL
{


Barrier::Barrier(std::size_t threadCount) :
    threadCount_ { threadCount },
    counter_     { threadCount }
{
}

void Barrier::Wait()
{
    std::unique_lock<std::mutex> lock{ mutex_ };

    if (--counter_ == 0)
    {
        counter_ = threadCount_;
        var_.notify_all();
    }
    else
        var_.wait(lock, [this]{ return (counter_ == threadCount_); });
}


} // /namespace LLGL



// ================================================================================
