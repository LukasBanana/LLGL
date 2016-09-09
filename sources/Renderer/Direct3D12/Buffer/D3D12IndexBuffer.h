/*
 * D3D12IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_INDEX_BUFFER_H__
#define __LLGL_D3D12_INDEX_BUFFER_H__


#include <LLGL/IndexBuffer.h>
#include "D3D12HardwareBuffer.h"


namespace LLGL
{


class D3D12IndexBuffer : public IndexBuffer
{

    public:

        D3D12HardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
