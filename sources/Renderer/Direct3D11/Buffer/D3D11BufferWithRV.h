/*
 * D3D11BufferWithRV.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_BUFFER_WITH_RV_H
#define LLGL_D3D11_BUFFER_WITH_RV_H


#include "D3D11Buffer.h"


namespace LLGL
{


// D3D11 buffer class with resource-views (SRV and UAV).
class D3D11BufferWithRV final : public D3D11Buffer
{

    public:

        void SetName(const char* name) override;

    public:

        D3D11BufferWithRV(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

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

        void CreateNativeSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);
        void CreateNativeUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements, UINT flags);

    private:

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

        UINT                                initialCount_   = std::numeric_limits<UINT>::max();

};


} // /namespace LLGL


#endif



// ================================================================================
