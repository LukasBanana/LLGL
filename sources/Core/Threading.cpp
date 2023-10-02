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


static constexpr unsigned g_maxThreadCountStaticArray = 64;

static void DoConcurrentRangeInWorkerContainer(
    const std::function<void(std::size_t begin, std::size_t end)>&  task,
    std::size_t                                                     count,
    std::size_t                                                     workerCount,
    std::thread*                                                    workers)
{
    /* Distribute work to threads */
    const std::size_t workSize          = count / workerCount;
    const std::size_t workSizeRemain    = count % workerCount;

    std::size_t offset = 0;

    for_range(i, workerCount)
    {
        workers[i] = std::thread(task, offset, offset + workSize);
        offset += workSize;
    }

    /* Execute task of remaining work on main thread */
    if (workSizeRemain > 0)
        task(offset, offset + workSizeRemain);

    /* Join worker threads */
    for_range(i, workerCount)
        workers[i].join();
}

LLGL_EXPORT void DoConcurrentRange(
    const std::function<void(std::size_t begin, std::size_t end)>&  task,
    std::size_t                                                     count,
    unsigned                                                        threadCount,
    unsigned                                                        threadMinWorkSize)
{
    if (threadCount == LLGL_MAX_THREAD_COUNT)
        threadCount = std::thread::hardware_concurrency();

    threadCount = std::min(threadCount, static_cast<unsigned>(count / threadMinWorkSize));

    if (threadCount <= 1)
    {
        /* Run single-threaded */
        task(0, count);
    }
    else if (threadCount <= g_maxThreadCountStaticArray)
    {
        /* Launch worker threads in static array */
        std::thread workers[g_maxThreadCountStaticArray];
        DoConcurrentRangeInWorkerContainer(task, count, threadCount, workers);
    }
    else if (threadCount > 1)
    {
        /* Launch worker threads in dynamic array */
        std::vector<std::thread> workers(threadCount);
        DoConcurrentRangeInWorkerContainer(task, count, threadCount, workers.data());
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
