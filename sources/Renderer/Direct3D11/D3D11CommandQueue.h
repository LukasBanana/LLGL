/*
 * D3D11CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_COMMAND_QUEUE_H
#define LLGL_D3D11_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <LLGL/ForwardDecls.h>
#include "RenderState/D3D11Fence.h"
#include "../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11QueryHeap;

class D3D11CommandQueue final : public CommandQueue
{

    public:

        D3D11CommandQueue(ID3D11Device* device, ComPtr<ID3D11DeviceContext>& context);

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

    private:

        bool QueryResultSingleUInt64(
            D3D11QueryHeap& queryHeapD3D,
            std::uint32_t   query,
            std::uint64_t&  data
        );

        bool QueryResultUInt32(
            D3D11QueryHeap& queryHeapD3D,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            std::uint32_t*  data
        );

        bool QueryResultUInt64(
            D3D11QueryHeap& queryHeapD3D,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            std::uint64_t*  data
        );

        bool QueryResultPipelineStatistics(
            D3D11QueryHeap&             queryHeapD3D,
            std::uint32_t               firstQuery,
            std::uint32_t               numQueries,
            QueryPipelineStatistics*    data
        );

    private:

        ComPtr<ID3D11DeviceContext> context_;
        D3D11Fence                  intermediateFence_;

};


} // /namespace LLGL


#endif



// ================================================================================
