/*
 * D3D12BufferConstantsPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUFFER_CONSTANTS_POOL_H
#define LLGL_D3D12_BUFFER_CONSTANTS_POOL_H


#include "../D3D12Resource.h"
#include <d3d12.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <vector>


namespace LLGL
{


enum class D3D12BufferConstants
{
    ZeroUInt64 = 0, // Zero initialized buffer ange of size UINT64
};

struct D3D12BufferConstantsView
{
    ID3D12Resource* resource;
    UINT64          offset;
    UINT64          size;
};

class D3D12CommandContext;
class D3D12CommandQueue;
class D3D12StagingBufferPool;

// Pool manager for special buffer constants, e.g. zero initialized buffer ange.
class D3D12BufferConstantsPool
{

    public:

        D3D12BufferConstantsPool(const D3D12BufferConstantsPool&) = delete;
        D3D12BufferConstantsPool& operator = (const D3D12BufferConstantsPool&) = delete;

        // Returns the instance of this singleton.
        static D3D12BufferConstantsPool& Get();

        // Initializes the device object and creates the internal immutable buffer.
        void InitializeDevice(
            ID3D12Device*           device,
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12StagingBufferPool& stagingBufferPool
        );

        // Clears all internal resources of this buffer pool.
        void Clear();

        // Returns the buffer view for the specified constants.
        D3D12BufferConstantsView FetchConstantsView(const D3D12BufferConstants id);

    private:

        struct ConstantRange
        {
            UINT64 offset;
            UINT64 size;
        };

    private:

        D3D12BufferConstantsPool() = default;

        std::uint32_t* AllocConstants(
            const D3D12BufferConstants  id,
            std::size_t                 size,
            SmallVector<std::uint32_t>& data
        );

        template <typename T>
        T* AllocConstants(const D3D12BufferConstants id, SmallVector<std::uint32_t>& data)
        {
            static_assert(sizeof(T) % 4 == 0, "D3D12 constants pool must be 4 byte aligned");
            return reinterpret_cast<T*>(AllocConstants(id, sizeof(T), data));
        }

        void CreateImmutableBuffer(
            ID3D12Device*                   device,
            D3D12CommandContext&            commandContext,
            D3D12CommandQueue&              commandQueue,
            D3D12StagingBufferPool&         stagingBufferPool,
            const ArrayView<std::uint32_t>& data
        );

    private:

        D3D12Resource               resource_;
        std::vector<ConstantRange>  constants_;

};


} // /namespace LLGL


#endif



// ================================================================================
