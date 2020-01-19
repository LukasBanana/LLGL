/*
 * D3D12BufferConstantsPool.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_BUFFER_CONSTNATS_POOL_H
#define LLGL_D3D12_BUFFER_CONSTNATS_POOL_H


#include "../D3D12Resource.h"
#include <d3d12.h>
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
            D3D12StagingBufferPool& stagingBufferPool
        );

        // Clears all internal resources of this buffer pool.
        void Clear();

        // Returns the buffer view for the specified constants.
        D3D12BufferConstantsView FetchConstants(const D3D12BufferConstants id);

    private:

        struct ConstantRegister
        {
            UINT64 offset;
            UINT64 size;
        };

    private:

        D3D12BufferConstantsPool() = default;

        void RegisterConstants(
            const D3D12BufferConstants  id,
            UINT64                      value,
            UINT64                      count,
            std::vector<std::uint64_t>& data
        );

        void CreateImmutableBuffer(
            ID3D12Device*               device,
            D3D12CommandContext&        commandContext,
            D3D12StagingBufferPool&     stagingBufferPool,
            std::vector<std::uint64_t>& data
        );

    private:

        D3D12Resource                   resource_;
        std::vector<ConstantRegister>   registers_;

};


} // /namespace LLGL


#endif



// ================================================================================
