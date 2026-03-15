/*
 * D3D9State.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_STATE_H
#define LLGL_D3D9_STATE_H


#include <LLGL/Sampler.h>
#include "../Direct3D9.h"


namespace LLGL
{

    
struct D3D9SamplerState
{
    DWORD addressU      = 0;
    DWORD addressV      = 0;
    DWORD addressW      = 0;
    DWORD borderColor   = 0;
    DWORD magFilter     = 0;
    DWORD minFilter     = 0;
    DWORD mipFilter     = 0;
    DWORD mipMapLodBias = 0;
    DWORD maxMipLevel   = 0;
    DWORD maxAnisotropy = 0;
};

struct D3DStreamSource
{
    IDirect3DVertexBuffer9* vertexBuffer;
    UINT                    offset;
    UINT                    stride;
};


} // /namespace LLGL


#endif



// ================================================================================
