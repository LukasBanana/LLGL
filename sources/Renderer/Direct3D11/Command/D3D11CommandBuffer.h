/*
 * D3D11CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_BUFFER_H
#define LLGL_D3D11_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "../../DXCommon/ComPtr.h"
#include "../../DXCommon/DXCore.h"
#include "../Direct3D11.h"
#include <dxgi.h>
#include <vector>
#include <cstddef>


namespace LLGL
{


class D3D11Buffer;
class D3D11StateManager;
class D3D11BindingTable;
class D3D11RenderTarget;
class D3D11SwapChain;
class D3D11RenderPass;
class D3D11PipelineState;
class D3D11PipelineLayout;
class D3D11ConstantsCache;
class D3D11RenderTargetHandles;

class D3D11CommandBuffer : public CommandBuffer
{

    public:

        D3D11CommandBuffer(bool isSecondaryCmdBuffer);

    public:

        // Returns true if this is a secondary command buffer that can be executed within a primary command buffer.
        inline bool IsSecondaryCmdBuffer() const
        {
            return isSecondaryCmdBuffer_;
        }

    private:

        const bool isSecondaryCmdBuffer_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
