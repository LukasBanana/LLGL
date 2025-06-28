/*
 * D3D9ConstantBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_CONSTANT_BUFFER_H
#define LLGL_D3D9_CONSTANT_BUFFER_H


#include "D3D9Buffer.h"
#include <vector>


namespace LLGL
{


// D3D9 constant buffer is emulated to set multiple shader constants at once.
class D3D9ConstantBuffer final : public D3D9Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        D3D9ConstantBuffer(const BufferDescriptor& desc, const void* initialData);

    private:

        struct ShaderRegister
        {
            std::uint32_t words[4];
        };

    private:

        std::vector<ShaderRegister> registers;

};


} // /namespace LLGL


#endif



// ================================================================================
