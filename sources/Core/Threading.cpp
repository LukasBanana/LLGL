/*
 * Threading.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Threading.h"
#include <LLGL/Utils/ForRange.h>
#include <thread>
#include <vector>
#include <algorithm>


namespace LLGL
{


LLGL_EXPORT void DoConcurrentRange(
    const std::function<void(std::size_t begin, std::size_t end)>&  task,
    std::size_t                                                     count,
    unsigned                                                        threadCount,
    unsigned                                                        threadMinWorkSize)
{
    if (threadCount >= Constants::maxThreadCount)
        threadCount = std::thread::hardware_concurrency();

    threadCount = std::min(threadCount, static_cast<unsigned>(count / threadMinWorkSize));

    if (threadCount > 1)
    {
        /* Launch worker threads */
        std::vector<std::thread> workers(threadCount);

        const std::size_t workSize          = count / threadCount;
        const std::size_t workSizeRemain    = count % threadCount;

        std::size_t offset = 0;

        for_range(i, threadCount)
        {
            workers[i] = std::thread(task, offset, offset + workSize);
            offset += workSize;
        }

        /* Execute task of remaining work on main thread */
        if (workSizeRemain > 0)
            task(offset, offset + workSizeRemain);

        /* Join worker threads */
        for (auto& w : workers)
            w.join();
    }
    else
    {
        /* Run single-threaded */
        task(0, count);
    }
}

LLGL_EXPORT void DoConcurrent(
    const std::function<void(std::size_t index)>&   task,
    std::size_t                                     count,
    unsigned                                        threadCount,
    unsigned                                        threadMinWorkSize)
{
    DoConcurrentRange(
        [&task](std::size_t begin, std::size_t end)
        {
            for_subrange(i, begin, end)
                task(i);
        },
        count,
        threadCount,
        threadMinWorkSize
    );
}


} // /namespace LLGL



// ================================================================================
