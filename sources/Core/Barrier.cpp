/*
 * Barrier.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    std::unique_lock<std::mutex> lock { mutex_ };

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
