/*
 * D3D12CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_QUEUE_H
#define LLGL_D3D12_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12CommandQueue : public CommandQueue
{

    public:

        D3D12CommandQueue(ComPtr<ID3D12CommandQueue>& queue, ComPtr<ID3D12CommandAllocator>& commandAlloc);

        /* ----- Command queues ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitForFence(Fence& fence, std::uint64_t timeout) override;
        void WaitForFinish() override;

        /* ----- Extended functions ----- */

        inline ID3D12CommandQueue* GetDxCommandQueue() const
        {
            return queue_.Get();
        }

    private:

        ComPtr<ID3D12CommandQueue>      queue_;
        ComPtr<ID3D12CommandAllocator>  commandAlloc_;

};


} // /namespace LLGL


#endif



// ================================================================================
