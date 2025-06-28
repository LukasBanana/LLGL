/*
 * D3D9Fence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_FENCE_H
#define LLGL_D3D9_FENCE_H


#include <LLGL/Fence.h>
#include <string>
#include <atomic>
#include <cstdint>


namespace LLGL
{


class D3D9Fence final : public Fence
{

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9Fence(std::uint64_t initialSignal = 0);

        void Signal(std::uint64_t signal);

        void WaitForSignal(std::uint64_t signal);

    private:

        std::string             label_;
        std::atomic_uint64_t    signal_;

};


} // /namespace LLGL


#endif



// ================================================================================
