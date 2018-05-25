/*
 * D3D11Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_TEXTURE_H
#define LLGL_D3D11_TEXTURE_H


#include <LLGL/Texture.h>
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


union D3D11HardwareTexture
{
    D3D11HardwareTexture() :
        resource { nullptr }
    {
    }
    ~D3D11HardwareTexture()
    {
    }

    ComPtr<ID3D11Resource>  resource;
    ComPtr<ID3D11Texture1D> tex1D;
    ComPtr<ID3D11Texture2D> tex2D;
    ComPtr<ID3D11Texture3D> tex3D;
};


class D3D11Texture : public Texture
{

    public:

        D3D11Texture(const TextureType type);

        Extent3D QueryMipLevelSize(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

        /* ----- Extended internal functions ---- */

        void CreateTexture1D(
            ID3D11Device* device,
            const D3D11_TEXTURE1D_DESC& desc,
            const D3D11_SUBRESOURCE_DATA* initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr
        );

        void CreateTexture2D(
            ID3D11Device* device,
            const D3D11_TEXTURE2D_DESC& desc,
            const D3D11_SUBRESOURCE_DATA* initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr
        );

        void CreateTexture3D(
            ID3D11Device* device,
            const D3D11_TEXTURE3D_DESC& desc,
            const D3D11_SUBRESOURCE_DATA* initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr
        );

        void UpdateSubresource(
            ID3D11DeviceContext* context,
            UINT mipSlice, UINT arraySlice, const D3D11_BOX& dstBox,
            const SrcImageDescriptor& imageDesc,
            std::size_t threadCount
        );

        // Creates a copy of the specified subresource of the hardware texture with CPU read access.
        void CreateSubresourceCopyWithCPUAccess(
            ID3D11Device* device, ID3D11DeviceContext* context,
            D3D11HardwareTexture& textureCopy,
            UINT cpuAccessFlags,
            UINT mipLevel
        ) const;

        // Creates a shader-resource-view (SRV) of a subresource of this texture object.
        void CreateSubresourceSRV(
            ID3D11Device* device,
            ID3D11ShaderResourceView** srvOutput,
            UINT baseMipLevel,
            UINT numMipLevels,
            UINT baseArrayLayer,
            UINT numArrayLayers
        );

        /* ----- Hardware texture objects ----- */

        // Returns the union of D3D hardware textures.
        inline const D3D11HardwareTexture& GetHwTexture() const
        {
            return hwTexture_;
        }

        // Returns the standard shader-resource-view (SRV) of the hardware texture object (full view of all layers and MIP levels).
        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        /* ----- Hardware texture parameters ----- */

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

        // Returns the number of array layers.
        inline UINT GetNumArrayLayers() const
        {
            return numArrayLayers_;
        }

    private:

        void CreateDefaultSRV(ID3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr);

        void SetResourceParams(DXGI_FORMAT format, const Extent3D& size, UINT arraySize);

        D3D11HardwareTexture                hwTexture_;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        //ComPtr<ID3D11UnorderedAccessView>   uav_; //TODO: use this to support UAV of textures

        DXGI_FORMAT                         format_             = DXGI_FORMAT_UNKNOWN;
        UINT                                numMipLevels_       = 0;
        UINT                                numArrayLayers_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
