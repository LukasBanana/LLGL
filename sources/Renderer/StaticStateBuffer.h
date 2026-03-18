/*
 * StaticStateBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STATIC_STATE_BUFFER_H
#define LLGL_STATIC_STATE_BUFFER_H


#include <LLGL/Export.h>
#include <LLGL/Report.h>
#include <LLGL/Constants.h>
#include "../Core/ByteBufferIterator.h"
#include <memory>
#include <cstdint>


namespace LLGL
{


// Manages the memory of static graphics pipeline state. Currently only viewports and scissors.
class LLGL_EXPORT StaticStateBuffer
{

    public:

        // Allocates the internal buffer for the static viewports and scissors of the specified graphics PSO descriptor.
        ByteBufferIterator Allocate(
            std::size_t     numViewports,
            std::size_t     numScissors,
            std::size_t     sizePerViewport,
            std::size_t     sizePerScissor,
            Report&         report,
            std::uint32_t   maxCount = LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS
        );

        inline ByteBufferConstIterator GetBufferIterator() const
        {
            return ByteBufferConstIterator{ buffer_.get() };
        }

        inline std::uint32_t GetNumViewports() const
        {
            return numViewports_;
        }

        inline std::uint32_t GetNumScissors() const
        {
            return numScissors_;
        }

        inline bool IsValid() const
        {
            return (numViewports_ > 0 || numScissors_ > 0);
        }

    public:

        inline operator bool () const
        {
            return IsValid();
        }

    private:

        std::uint32_t                       numViewports_   = 0;
        std::uint32_t                       numScissors_    = 0;
        std::unique_ptr<std::uint32_t[]>    buffer_; // Raw buffer with 32-bit alignment

};


} // /namespace LLGL


#endif // /LLGL_BUILD_STATIC_LIB



// ================================================================================
