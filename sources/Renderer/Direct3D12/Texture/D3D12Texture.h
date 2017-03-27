/*
 * D3D12Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        Gs::Vector3ui QueryMipLevelSize(unsigned int mipLevel) const override;

        /* ----- Extended internal functions ---- */

        void UpdateSubresource(
            ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
            ComPtr<ID3D12Resource>& uploadBuffer, D3D12_SUBRESOURCE_DATA& subresourceData
        );

        //! Returns the ID3D12Resource object.
        inline ID3D12Resource* Get() const
        {
            return resource_.Get();
        }

        // Returns the descriptor heap for shader-resource-views (SRV).
        inline ID3D12DescriptorHeap* GetDescriptorHeap() const
        {
            return descHeap_.Get();
        }

        // Returns the hardware resource format.
        inline DXGI_FORMAT GetFormat() const
        {
            return format_;
        }

        // Returns the number of MIP-map levels.
        inline UINT GetNumMipLevels() const
        {
            return numMipLevels_;
        }

    private:

        void CreateResource(
            ID3D12Device* device, const D3D12_RESOURCE_DESC& desc, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr
        );

        ComPtr<ID3D12Resource>          resource_;
        ComPtr<ID3D12DescriptorHeap>    descHeap_; // descriptor heap for shader resource views (SRV)

        DXGI_FORMAT                     format_         = DXGI_FORMAT_UNKNOWN;
        UINT                            numMipLevels_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
