/*
 * D3D11CommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_QUEUE_H
#define LLGL_D3D11_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <LLGL/ForwardDecls.h>
#include "../RenderState/D3D11Fence.h"
#include "../RenderState/D3D11StateManager.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11QueryHeap;

class D3D11CommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        D3D11CommandQueue(
            ID3D11Device*                               device,
            ComPtr<ID3D11DeviceContext>&                context,
            const std::shared_ptr<D3D11StateManager>&   stateMngr
        );

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

        ComPtr<ID3D11DeviceContext>         context_;
        std::shared_ptr<D3D11StateManager>  stateMngr_;
        D3D11Fence                          intermediateFence_;

};


} // /namespace LLGL


#endif



// ================================================================================
