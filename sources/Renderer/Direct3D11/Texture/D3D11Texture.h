/*
 * D3D11Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_TEXTURE_H
#define LLGL_D3D11_TEXTURE_H


#include <LLGL/Texture.h>
#include <LLGL/ImageFlags.h>
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


// Union for easy handling of native D3D11 texture objects.
union D3D11NativeTexture
{
    inline D3D11NativeTexture() :
        resource { nullptr }
    {
    }
    inline D3D11NativeTexture(const D3D11NativeTexture& rhs) :
        resource { rhs.resource }
    {
    }
    inline D3D11NativeTexture& operator = (const D3D11NativeTexture& rhs)
    {
        resource = rhs.resource;
        return *this;
    }
    inline ~D3D11NativeTexture()
    {
        resource.Reset();
    }

    ComPtr<ID3D11Resource>  resource;
    ComPtr<ID3D11Texture1D> tex1D;
    ComPtr<ID3D11Texture2D> tex2D;
    ComPtr<ID3D11Texture3D> tex3D;
};


class D3D11Texture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

        Format GetFormat() const override;

    public:

        D3D11Texture(const TextureDescriptor& desc);

        /* ----- Extended internal functions ---- */

        void CreateTexture1D(
            ID3D11Device*                           device,
            const TextureDescriptor&                desc,
            const D3D11_SUBRESOURCE_DATA*           initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc     = nullptr,
            const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc     = nullptr
        );

        void CreateTexture2D(
            ID3D11Device*                           device,
            const TextureDescriptor&                desc,
            const D3D11_SUBRESOURCE_DATA*           initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc     = nullptr,
            const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc     = nullptr
        );

        void CreateTexture3D(
            ID3D11Device*                           device,
            const TextureDescriptor&                desc,
            const D3D11_SUBRESOURCE_DATA*           initialData = nullptr,
            const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc     = nullptr,
            const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc     = nullptr
        );

        void UpdateSubresource(
            ID3D11DeviceContext*        context,
            UINT                        mipLevel,
            UINT                        arrayLayer,
            const D3D11_BOX&            region,
            const SrcImageDescriptor&   imageDesc,
            std::size_t                 threadCount
        );

        // Creates a copy of the specified subresource of the hardware texture with CPU read access.
        void CreateSubresourceCopyWithCPUAccess(
            ID3D11Device*           device,
            ID3D11DeviceContext*    context,
            D3D11NativeTexture&     textureOutput,
            UINT                    cpuAccessFlags,
            const TextureRegion&    region
        );

        // Creates an uninitialized copy of the specified subresource of the hardware texture with an equivalent unsigned integer format.
        void CreateSubresourceCopyWithUIntFormat(
            ID3D11Device*               device,
            D3D11NativeTexture&         textureOutput,
            ID3D11ShaderResourceView**  srvOutput,
            ID3D11UnorderedAccessView** uavOutput,
            const TextureRegion&        region,
            const TextureType           subresourceType
        );

        // Creates a depth-stencil-view (DSV) of a subresource of this texture object.
        void CreateSubresourceDSV(
            ID3D11Device*               device,
            ID3D11DepthStencilView**    dsvOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        baseMipLevel,
            UINT                        baseArrayLayer,
            UINT                        numArrayLayers
        );

        // Creates a shader-resource-view (SRV) of a subresource of this texture object.
        void CreateSubresourceSRV(
            ID3D11Device*               device,
            ID3D11ShaderResourceView**  srvOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        baseMipLevel,
            UINT                        numMipLevels,
            UINT                        baseArrayLayer,
            UINT                        numArrayLayers
        );

        // Creates an unordered-access-view (UAV) of a subresource of this texture object.
        void CreateSubresourceUAV(
            ID3D11Device*               device,
            ID3D11UnorderedAccessView** uavOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        baseMipLevel,
            UINT                        baseArrayLayer,
            UINT                        numArrayLayers
        );

        // Returns the subresource index for the specified MIP-map level and array layer.
        UINT CalcSubresource(UINT mipLevel, UINT arrayLayer) const;

        // Returns the subresource index for the specified texture location with respect to the type of this texture (i.e. whether or not array layers are included).
        UINT CalcSubresource(const TextureLocation& location) const;

        // Returns the texture region for the specified offset and extent with respect to the type of this texture (i.e. whether or not array layers are handled by the subresource index).
        D3D11_BOX CalcRegion(const Offset3D& offset, const Extent3D& extent) const;

        /* ----- Hardware texture objects ----- */

        // Returns the native D3D texture object.
        inline const D3D11NativeTexture& GetNative() const
        {
            return native_;
        }

        // Returns the standard shader resource view (SRV) of the hardware texture object (full view of all layers and MIP levels).
        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        // Returns the standard unordered access view (UAV) of the hardware texture object (full view of all layers and MIP levels).
        inline ID3D11UnorderedAccessView* GetUAV() const
        {
            return uav_.Get();
        }

        /* ----- Hardware texture parameters ----- */

        // Returns the hardware resource format.
        inline DXGI_FORMAT GetDXFormat() const
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

        void CreateDefaultResourceViews(
            ID3D11Device*                           device,
            const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc,
            const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc,
            long                                    bindFlags
        );

        void CreateDefaultSRV(ID3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr);
        void CreateDefaultUAV(ID3D11Device* device, const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr);

        void SetResourceParams(DXGI_FORMAT format, const Extent3D& extent, UINT mipLevels, UINT arraySize);

    private:

        D3D11NativeTexture                  native_;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

        DXGI_FORMAT                         format_             = DXGI_FORMAT_UNKNOWN;
        UINT                                numMipLevels_       = 0;
        UINT                                numArrayLayers_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
