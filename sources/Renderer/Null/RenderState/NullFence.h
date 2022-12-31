/*
 * NullFence.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_FENCE_H
#define LLGL_NULL_FENCE_H


#include <LLGL/Fence.h>
#include <string>
#include <atomic>


namespace LLGL
{


class NullFence final : public Fence
{

    public:

        void SetName(const char* name) override;

        void Signal(std::uint64_t signal);

        void WaitForSignal(std::uint64_t signal);

    private:

        std::string             label_;
        std::atomic_uint64_t    signal_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
