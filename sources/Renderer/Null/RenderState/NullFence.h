/*
 * NullFence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_FENCE_H
#define LLGL_NULL_FENCE_H


#include <LLGL/Fence.h>
#include <string>
#include <atomic>
#include <cstdint>


namespace LLGL
{


class NullFence final : public Fence
{

    public:

        void SetDebugName(const char* name) override;

    public:

        NullFence(std::uint64_t initialSignal = 0);

        void Signal(std::uint64_t signal);

        void WaitForSignal(std::uint64_t signal);

    private:

        std::string             label_;
        std::atomic_uint64_t    signal_;

};


} // /namespace LLGL


#endif



// ================================================================================
