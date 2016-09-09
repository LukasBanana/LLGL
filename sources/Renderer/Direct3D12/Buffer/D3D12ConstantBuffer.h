/*
 * D3D12ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_CONSTANT_BUFFER_H__
#define __LLGL_D3D12_CONSTANT_BUFFER_H__


#include <LLGL/ConstantBuffer.h>
#include "D3D12HardwareBuffer.h"


namespace LLGL
{


class D3D12ConstantBuffer : public ConstantBuffer
{

    public:

        D3D12HardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
