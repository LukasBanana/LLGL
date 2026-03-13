/*
 * D3D9RasterizerState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_RASTERIZER_STATE_H
#define LLGL_D3D9_RASTERIZER_STATE_H


#include <LLGL/ForwardDecls.h>
#include "../Direct3D9.h"
#include <memory>


namespace LLGL
{


class D3D9RasterizerState;
class D3D9StateManager;

using D3D9RasterizerStateSPtr = std::shared_ptr<D3D9RasterizerState>;

class D3D9RasterizerState
{

    public:

        D3D9RasterizerState() = default;

        D3D9RasterizerState(const RasterizerDescriptor& desc);

        // Binds the entire rasterizer state.
        void Bind(D3D9StateManager& stateMngr);

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const D3D9RasterizerState& lhs, const D3D9RasterizerState& rhs);

    private:

        BOOL        scissorTestEnable_  : 1;                    // D3DRS_SCISSORTESTENABLE
        D3DCULL     cullMode_               = D3DCULL_CCW;      // D3DRS_CULLMODE
        D3DFILLMODE fillMode_               = D3DFILL_SOLID;    // D3DRS_FILLMODE
        DWORD       depthBias_              = 0;                // D3DRS_DEPTHBIAS
        DWORD       slopeScaleDepthBias_    = 0;                // D3DRS_SLOPESCALEDEPTHBIAS

};


} // /namespace LLGL


#endif



// ================================================================================
