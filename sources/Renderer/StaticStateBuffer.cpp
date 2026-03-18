/*
 * StaticStateBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "StaticStateBuffer.h"
#include <LLGL/PipelineStateFlags.h>
#include "../Core/CoreUtils.h"


namespace LLGL
{


ByteBufferIterator StaticStateBuffer::Allocate(
    std::size_t     numViewports,
    std::size_t     numScissors,
    std::size_t     sizePerViewport,
    std::size_t     sizePerScissor,
    Report&         report,
    std::uint32_t   maxCount)
{
    /* Store number of viewports and validate limit */
    numViewports_ = static_cast<std::uint32_t>(numViewports);
    if (numViewports_ > maxCount)
    {
        report.Errorf("too many viewports in graphics pipeline state: %u specified, but limit is %d\n", numViewports_, maxCount);
        numViewports_ = maxCount;
    }

    /* Store number of scissors and validate limit */
    numScissors_ = static_cast<std::uint32_t>(numScissors);
    if (numScissors_ > maxCount)
    {
        report.Errorf("too many scissor rectangles in graphics pipeline state: %u specified, but limit is %d\n", numScissors_, maxCount);
        numScissors_ = maxCount;
    }

    /* Ensure both have the same number */
    if (numViewports_ > 0 && numScissors_ > 0 && numViewports_ != numScissors_)
    {
        report.Errorf("mismatch between number of viewports (%u) and scissors (%u) in graphics pipeline state", numViewports_, numScissors_);
        const std::uint32_t minCount = std::min<std::uint32_t>(numViewports_, numScissors_);
        numViewports_ = minCount;
        numScissors_  = minCount;
    }

    /* Allocate internal buffer for static state of viewports and scissors */
    const std::size_t bufferSize =
    (
        numViewports_ * sizePerViewport +
        numScissors_  * sizePerScissor
    );
    buffer_ = MakeUniqueArray<std::uint32_t>(DivideRoundUp(bufferSize, sizeof(std::uint32_t)));

    return ByteBufferIterator{ reinterpret_cast<char*>(buffer_.get()) };
}


} // /namespace LLGL



// ================================================================================
