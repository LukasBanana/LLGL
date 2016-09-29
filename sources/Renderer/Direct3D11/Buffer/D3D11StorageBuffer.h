/*
 * D3D11StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_STORAGE_BUFFER_H__
#define __LLGL_D3D11_STORAGE_BUFFER_H__


#include <LLGL/StorageBuffer.h>
#include <LLGL/RenderSystemFlags.h>
#include "D3D11HardwareBuffer.h"


namespace LLGL
{


class D3D11StorageBuffer : public StorageBuffer
{

    public:

        void CreateResource(
            ID3D11Device* device, UINT bufferSize, const BufferUsage usage,
            const StorageBufferType type, const void* initialData = nullptr
        );

        bool IsUAV() const;
        bool IsStructured() const;
        bool IsByteAddressable() const;

        D3D11HardwareBuffer hwBuffer;

    private:

        void CreateUAV(ID3D11Device* device);
        void CreateSRV(ID3D11Device* device);

        StorageBufferType                   type_   = StorageBufferType::Buffer;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

};


} // /namespace LLGL


#endif



// ================================================================================
