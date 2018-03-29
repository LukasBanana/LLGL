/*
 * D3D11CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_COMMAND_QUEUE_H
#define LLGL_D3D11_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include "../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11CommandQueue : public CommandQueue
{

    public:

        D3D11CommandQueue(ComPtr<ID3D11DeviceContext>& context);

        /* ----- Command queues ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitForFence(Fence& fence, std::uint64_t timeout) override;
        void WaitForFinish() override;

    private:

        ComPtr<ID3D11DeviceContext> context_;

};


} // /namespace LLGL


#endif



// ================================================================================
