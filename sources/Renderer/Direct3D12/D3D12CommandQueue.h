/*
 * D3D12CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_QUEUE_H
#define LLGL_D3D12_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "RenderState/D3D12Fence.h"
#include "../DXCommon/ComPtr.h"
#include "../StaticLimits.h"
#include <d3d12.h>
#include <cstddef>


namespace LLGL
{


class D3D12RenderSystem;

class D3D12CommandQueue final : public CommandQueue
{

    public:

        D3D12CommandQueue(D3D12RenderSystem& renderSystem);

        /* ----- Command Buffers ----- */

        void Begin(CommandBuffer& commandBuffer, long flags = 0) override;
        void End(CommandBuffer& commandBuffer) override;

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

        /* ----- Extended functions ----- */

        // Returns the native ID3D12CommandQueue object.
        inline ID3D12CommandQueue* GetNative() const
        {
            return cmdQueue_.Get();
        }

    private:

        void NextCmdAllocator();
        void ResetCommandList(ID3D12GraphicsCommandList* commandList);

        inline ID3D12CommandAllocator* GetCmdAllocator() const
        {
            return cmdAllocators_[currentCmdAllocator_].Get();
        }

        static const std::size_t g_numCmdAllocators = 3;

        ComPtr<ID3D12CommandQueue>      cmdQueue_;
        ComPtr<ID3D12CommandAllocator>  cmdAllocators_[g_numCmdAllocators];
        std::size_t                     currentCmdAllocator_                = 0;

        D3D12Fence                      intermediateFence_;

};


} // /namespace LLGL


#endif



// ================================================================================
