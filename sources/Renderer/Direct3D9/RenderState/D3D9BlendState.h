/*
 * D3D9BlendState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_BLEND_STATE_H
#define LLGL_D3D9_BLEND_STATE_H


#include <LLGL/ForwardDecls.h>
#include "../Direct3D9.h"
#include <memory>


namespace LLGL
{


class D3D9StateManager;
class D3D9BlendState;

using D3D9BlendStateSPtr = std::shared_ptr<D3D9BlendState>;

class D3D9BlendState
{

    public:

        D3D9BlendState() = default;

        D3D9BlendState(const BlendDescriptor& desc);

        // Binds the entire blend state.
        void Bind(D3D9StateManager& stateMngr);

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const D3D9BlendState& lhs, const D3D9BlendState& rhs);

    private:

        BOOL        isBlendFactorEnabled_           : 1;
        BOOL        isAlphaBlendEnabled_            : 1;                    // D3DRS_ALPHABLENDENABLE
        BOOL        isSeparateAlphaBlendEnabled_    : 1;                    // D3DRS_SEPARATEALPHABLENDENABLE

        D3DCOLOR    blendFactor_                        = 0xFFFFFFFF;       // Requires D3DPBLENDCAPS_BLENDFACTOR caps

        D3DBLENDOP  blendOp_                            = D3DBLENDOP_ADD;
        D3DBLEND    srcBlend_                           = D3DBLEND_ONE;
        D3DBLEND    destBlend_                          = D3DBLEND_ZERO;

        D3DBLENDOP  blendOpAlpha_                       = D3DBLENDOP_ADD;
        D3DBLEND    srcBlendAlpha_                      = D3DBLEND_ONE;
        D3DBLEND    destBlendAlpha_                     = D3DBLEND_ZERO;

};


} // /namespace LLGL


#endif



// ================================================================================
