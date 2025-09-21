/*
 * D3D9Types.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_TYPES_H
#define LLGL_D3D9_TYPES_H


#include <LLGL/Format.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/SamplerFlags.h>
#include "Direct3D9.h"


namespace LLGL
{

namespace D3D9Types
{


D3DFORMAT ToD3DFormat( const Format format );
D3DFORMAT ToD3DIndexFormat( const Format format );
D3DDECLTYPE ToD3DDeclType( const Format format );
D3DTEXTUREADDRESS ToD3DTextureAddress( const SamplerAddressMode mode );
D3DCOLOR ToD3DColor( const float color[4] );
D3DTEXTUREFILTERTYPE ToD3DTextureFilter( const SamplerFilter type );

Format ToFormat(const D3DFORMAT format);

D3DPRIMITIVETYPE ToD3DPrimitiveType( const PrimitiveTopology topology );


} // /namespace D3D9Types

} // /namespace LLGL


#endif



// ================================================================================
