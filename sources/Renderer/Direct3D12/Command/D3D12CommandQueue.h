/*
 * D3D12CommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMMAND_QUEUE_H
#define LLGL_D3D12_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <LLGL/QueryHeapFlags.h>
#include "D3D12CommandContext.h"
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

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        D3D12CommandQueue(
            D3D12Device&            device,
            D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT
        );

        void SetDebugName(const char* name) override;

    public:

        // Submits the specified fence with a custom value.
        void SignalFence(ID3D12Fence* fence, UINT64 value);

        // Executes the specified command lists.
        void ExecuteCommandLists(UINT numCommandsLists, ID3D12CommandList* const* commandLists);
        void ExecuteCommandList(ID3D12CommandList* commandList);

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

    private:

        void DetermineTimestampFrequency();

        void QueryResultSingleUInt64(
            QueryType           queryType,
            const void*         mappedData,
            std::uint32_t       query,
            std::uint64_t&      data
        );

        void QueryResultUInt32(
            QueryType           queryType,
            const void*         mappedData,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            std::uint32_t*      data
        );

        void QueryResultUInt64(
            QueryType           queryType,
            const void*         mappedData,
            std::uint32_t       firstQuery,
            std::uint32_t       numQueries,
            std::uint64_t*      data
        );

        bool QueryResultPipelineStatistics(
            QueryType                   queryType,
            const void*                 mappedData,
            std::uint32_t               firstQuery,
            std::uint32_t               numQueries,
            QueryPipelineStatistics*    data
        );

    private:

        ComPtr<ID3D12CommandQueue>  native_;
        D3D12CommandContext         commandContext_;
        D3D12NativeFence            queueFence_;
        UINT64                      queueFenceValue_        = 0;
        double                      timestampScale_         = 1.0;  // Frequency to nanoseconds scale
        bool                        isTimestampNanosecs_    = true; // True, if timestamps are in nanoseconds unit
        bool                        busy_                   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
