/*
 * D3D12Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_TEXTURE_H
#define LLGL_D3D12_TEXTURE_H


#include <LLGL/Texture.h>
#include <d3d12.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D12Texture : public Texture
{

    public:

        D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc);

        Extent3D QueryMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

        /* ----- Extended internal functions ---- */

        void UpdateSubresource(
            ID3D12Device*               device,
            ID3D12GraphicsCommandList*  commandList,
            ComPtr<ID3D12Resource>&     uploadBuffer,
            D3D12_SUBRESOURCE_DATA&     subresourceData,
            UINT                        firstArrayLayer = 0,
            UINT                        numArrayLayers  = ~0
        );

        void CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle);

        //! Returns the native ID3D12Resource object.
        inline ID3D12Resource* GetNative() const
        {
            return resource_.Get();
        }

        // Returns the hardware resource format.
        inline DXGI_FORMAT GetFormat() const
        {
            return format_;
        }

        // Returns the number of MIP-map levels specified at creation time.
        inline UINT GetNumMipLevels() const
        {
            return numMipLevels_;
        }

        // Returns the number of array layers of the hardware resource (for cube textures, this is a multiple of 6)
        inline UINT GetNumArrayLayers() const
        {
            return numArrayLayers_;
        }

    private:

        void CreateResource(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc);

        ComPtr<ID3D12Resource>  resource_;

        DXGI_FORMAT             format_         = DXGI_FORMAT_UNKNOWN;
        UINT                    numMipLevels_   = 0;
        UINT                    numArrayLayers_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
