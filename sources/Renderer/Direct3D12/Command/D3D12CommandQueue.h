/*
 * D3D12CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_QUEUE_H
#define LLGL_D3D12_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "D3D12CommandContext.h"
#include <LLGL/StaticLimits.h>
#include "../RenderState/D3D12Fence.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <cstddef>


namespace LLGL
{


class D3D12Device;

class D3D12CommandQueue final : public CommandQueue
{

    public:

        D3D12CommandQueue(
            D3D12Device&            device,
            D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT
        );

        void SetName(const char* name) override;

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Queries ----- */

        bool QueryResult(
            QueryHeap&      queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        ) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

    public:

        // Submits the specified fence with a custom value.
        void SignalFence(D3D12Fence& fenceD3D, UINT64 value);

        // Returns the native ID3D12CommandQueue object.
        inline ID3D12CommandQueue* GetNative() const
        {
            return native_.Get();
        }

        // Returns the command context for this queue.
        inline D3D12CommandContext& GetContext()
        {
            return commandContext_;
        }

        // Returns the global fence object for this queue.
        inline D3D12Fence& GetGlobalFence()
        {
            return globalFence_;
        }

        // Returns the global fence object for this queue as constant reference.
        inline const D3D12Fence& GetGlobalFence() const
        {
            return globalFence_;
        }

    private:

        void DetermineTimestampFrequency();

        void QueryResultSingleUInt64(
            D3D12_QUERY_TYPE    queryType,
            const void*         mappedData,
            std::uint32_t       query,
            std::uint64_t&      data
        );

        void QueryResultUInt32(
            D3D12_QUERY_TYPE    queryType,
            const void*         mappedData,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            std::uint32_t*      data
        );

        void QueryResultUInt64(
            D3D12_QUERY_TYPE    queryType,
            const void*         mappedData,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            std::uint64_t*      data
        );

        bool QueryResultPipelineStatistics(
            D3D12_QUERY_TYPE            queryType,
            const void*                 mappedData,
            std::uint32_t               firstQuery,
            std::uint32_t               numQueries,
            QueryPipelineStatistics*    data
        );

    private:

        ComPtr<ID3D12CommandQueue>  native_;
        D3D12CommandContext         commandContext_;
        D3D12Fence                  globalFence_;
        double                      timestampScale_         = 1.0;  // Frequency to nanoseconds scale
        bool                        isTimestampNanosecs_    = true; // True, if timestamps are in nanoseconds unit

};


} // /namespace LLGL


#endif



// ================================================================================
