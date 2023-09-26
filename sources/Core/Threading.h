/*
 * Threading.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_THREADING_H
#define LLGL_THREADING_H


#include <LLGL/Export.h>
#include <LLGL/Constants.h>
#include <functional>
#include <cstddef>


namespace LLGL
{


LLGL_EXPORT void DoConcurrentRange(
    const std::function<void(std::size_t begin, std::size_t end)>&  task,
    std::size_t                                                     count,
    unsigned                                                        threadCount         = LLGL_MAX_THREAD_COUNT,
    unsigned                                                        threadMinWorkSize   = 64
);

LLGL_EXPORT void DoConcurrent(
    const std::function<void(std::size_t index)>&   task,
    std::size_t                                     count,
    unsigned                                        threadCount         = LLGL_MAX_THREAD_COUNT,
    unsigned                                        threadMinWorkSize   = 64
);


} // /namespace LLGL


#endif



// ================================================================================
