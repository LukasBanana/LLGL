/*
 * DXTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DX_TYPES_H__
#define __LLGL_DX_TYPES_H__


#include <LLGL/VertexAttribute.h>
#include <d3d12.h>


namespace LLGL
{

namespace DXTypes
{


DXGI_FORMAT Map( const VertexAttribute& attrib );


} // /namespace DXTypes

} // /namespace LLGL


#endif



// ================================================================================
