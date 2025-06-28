/*
 * D3D9VertexBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_VERTEX_BUFFER_H
#define LLGL_D3D9_VERTEX_BUFFER_H


#include "D3D9Buffer.h"
#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D9VertexBuffer final : public D3D9Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        D3D9VertexBuffer(IDirect3DDevice9* device, const BufferDescriptor& desc, const void* initialData);

        inline IDirect3DVertexBuffer9* GetNative() const
        {
            return d3dBuffer_.Get();
        }

        inline UINT GetStride() const
        {
            return stride_;
        }

    private:

        ComPtr<IDirect3DVertexBuffer9>  d3dBuffer_;
        UINT                            stride_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
