/*
 * D3D9Buffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_BUFFER_H
#define LLGL_D3D9_BUFFER_H


#include <LLGL/Buffer.h>
#include "../../../Core/Assertion.h"
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9Buffer : public Buffer
{

    public:

        void SetDebugName(const char* name) override;

    public:

        virtual HRESULT Write(UINT dstOffset, const void* data, UINT dataSize) = 0;

    protected:

        D3D9Buffer(long bindFlags);

        static DWORD GetUsageFlags(long bindFlags, long cpuAccessFlags, long miscFlags);

    protected:

        template <typename T>
        static HRESULT WriteLocked(T* d3dBuffer, UINT dstOffset, const void* data, UINT dataSize)
        {
            LLGL_ASSERT_PTR(data);

            void* outData = nullptr;
            HRESULT hr = d3dBuffer->Lock(dstOffset, dataSize, &outData, D3DLOCK_DISCARD);
            D3DThrowIfFailed(hr, "failed to lock D3D9 buffer");

            LLGL_ASSERT_PTR(outData);
            ::memcpy(outData, data, dataSize);

            hr = d3dBuffer->Unlock();
            D3DThrowIfFailed(hr, "failed to unlock D3D9 buffer");

            return S_OK;
        }

};


} // /namespace LLGL


#endif



// ================================================================================
