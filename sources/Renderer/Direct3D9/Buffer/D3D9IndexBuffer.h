/*
 * D3D9IndexBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_INDEX_BUFFER_H
#define LLGL_D3D9_INDEX_BUFFER_H


#include "D3D9Buffer.h"
#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D9IndexBuffer final : public D3D9Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        D3D9IndexBuffer(IDirect3DDevice9* device, const BufferDescriptor& desc);

        HRESULT Write(UINT dstOffset, const void* data, UINT dataSize) override;

        inline IDirect3DIndexBuffer9* GetNative() const
        {
            return d3dBuffer_.Get();
        }

    private:

        ComPtr<IDirect3DIndexBuffer9> d3dBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
