/*
 * D3D9DepthStencilState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_DEPTH_STENCIL_STATE_H
#define LLGL_D3D9_DEPTH_STENCIL_STATE_H


#include <LLGL/ForwardDecls.h>
#include "../Direct3D9.h"
#include <memory>


namespace LLGL
{


class D3D9DepthStencilState;
class D3D9StateManager;

using D3D9DepthStencilStateSPtr = std::shared_ptr<D3D9DepthStencilState>;

class D3D9DepthStencilState
{

    public:

        D3D9DepthStencilState() = default;

        D3D9DepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);

        // Binds the entire depth-stencil state.
        void Bind(D3D9StateManager& stateMngr);

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const D3D9DepthStencilState& lhs, const D3D9DepthStencilState& rhs);

    private:

        struct D3D9StencilFaceState
        {
            static int CompareSWO(const D3D9StencilFaceState& lhs, const D3D9StencilFaceState& rhs);

            D3DSTENCILOP    opFail  = D3DSTENCILOP_KEEP;    // D3DRS_STENCILFAIL
            D3DSTENCILOP    opZFail = D3DSTENCILOP_KEEP;    // D3DRS_STENCILZFAIL
            D3DSTENCILOP    opPass  = D3DSTENCILOP_KEEP;    // D3DRS_STENCILPASS
            D3DCMPFUNC      cmpFunc = D3DCMP_ALWAYS;        // D3DRS_STENCILFUNC
        };

    private:

        // Depth states
        BOOL                    twoSidedStencilMode_    : 1;                    // D3DRS_TWOSIDEDSTENCILMODE
        BOOL                    zEnable_                : 1;                    // D3DRS_ZENABLE (FALSE)
        BOOL                    zWriteEnable_           : 1;                    // D3DRS_ZWRITEENABLE (TRUE)
        D3DCMPFUNC              zFunc_                      = D3DCMP_LESSEQUAL; // D3DRS_ZFUNC

        // Stencil states
        BOOL                    stencilEnable_          = FALSE;            // D3DRS_STENCILENABLE
        DWORD                   stencilRef_             = 0;                    // D3DRS_STENCILREF
        DWORD                   stencilMask_            = 0xFFFFFFFF;           // D3DRS_STENCILMASK
        DWORD                   stencilWriteMask_       = 0xFFFFFFFF;           // D3DRS_STENCILWRITEMASK
        D3D9StencilFaceState    stencilFront_;
        D3D9StencilFaceState    stencilBack_;

};


} // /namespace LLGL


#endif



// ================================================================================
