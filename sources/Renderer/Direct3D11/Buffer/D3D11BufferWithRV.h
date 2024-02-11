/*
 * D3D11BufferWithRV.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BUFFER_WITH_RV_H
#define LLGL_D3D11_BUFFER_WITH_RV_H


#include "D3D11Buffer.h"
#include <limits.h>


namespace LLGL
{


// D3D11 buffer class with resource-views (SRV and UAV).
class D3D11BufferWithRV final : public D3D11Buffer
{

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D11BufferWithRV(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        /*
        Creates a shader-resource-view (SRV) of a subresource of this buffer object.
        If 'device' is null, the original device this buffer was created with will be used.
        */
        void CreateSubresourceSRV(
            ID3D11Device*               device,
            ID3D11ShaderResourceView**  srvOutput,
            DXGI_FORMAT                 format,
            UINT                        firstElement,
            UINT                        numElements
        );

        /*
        Creates an unordered-access-view (UAV) of a subresource of this buffer object.
        If 'device' is null, the original device this buffer was created with will be used.
        */
        void CreateSubresourceUAV(
            ID3D11Device*               device,
            ID3D11UnorderedAccessView** uavOutput,
            DXGI_FORMAT                 format,
            UINT                        firstElement,
            UINT                        numElements
        );

        // Returns the native SRV object.
        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        // Returns the native UAV object.
        inline ID3D11UnorderedAccessView* GetUAV() const
        {
            return uav_.Get();
        }

        // Returns the initial value for the internal buffer counter.
        inline UINT GetInitialCount() const
        {
            return initialCount_;
        }

    private:

        void CreateInternalSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);
        void CreateInternalUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);

    private:

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;
        UINT                                uavFlags_       = 0;
        UINT                                initialCount_   = UINT_MAX;

};


} // /namespace LLGL


#endif



// ================================================================================
