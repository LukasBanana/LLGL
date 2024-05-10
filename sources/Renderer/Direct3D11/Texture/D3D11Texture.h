/*
 * D3D11Texture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_TEXTURE_H
#define LLGL_D3D11_TEXTURE_H


#include <LLGL/Texture.h>
#include <LLGL/ImageFlags.h>
#include <d3d11.h>
#include "../RenderState/D3D11BindingLocator.h"
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class Report;

class D3D11Texture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D11Texture(ID3D11Device* device, const TextureDescriptor& desc);

        HRESULT UpdateSubresource(
            ID3D11DeviceContext*    context,
            UINT                    mipLevel,
            UINT                    baseArrayLayer,
            UINT                    numArrayLayers,
            const D3D11_BOX&        dstBox,
            const ImageView&        imageView,
            Report*                 report          = nullptr
        );

        // Creates a copy of the specified subresource of the hardware texture with CPU read access.
        void CreateSubresourceCopyWithCPUAccess(
            ID3D11Device*           device,
            ID3D11DeviceContext*    context,
            ComPtr<ID3D11Resource>& textureOutput,
            UINT                    cpuAccessFlags,
            const TextureRegion&    region
        );

        // Creates an uninitialized copy of the specified subresource of the hardware texture with an equivalent unsigned integer format.
        void CreateSubresourceCopyWithUIntFormat(
            ID3D11Device*               device,
            ComPtr<ID3D11Resource>&     textureOutput,
            ID3D11ShaderResourceView**  srvOutput,
            ID3D11UnorderedAccessView** uavOutput,
            const TextureRegion&        region,
            const TextureType           subresourceType
        );

        /*
        Creates a shader-resource-view (SRV) of a subresource of this texture object.
        If 'device' is null, the original device this texture was created with will be used.
        */
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

        /*
        Creates an unordered-access-view (UAV) of a subresource of this texture object.
        If 'device' is null, the original device this texture was created with will be used.
        */
        void CreateSubresourceUAV(
            ID3D11Device*               device,
            ID3D11UnorderedAccessView** uavOutput,
            const TextureType           type,
            const DXGI_FORMAT           format,
            UINT                        mipLevel,
            UINT                        baseArrayLayerOrSlice,
            UINT                        numArrayLayersOrSlices
        );

        // Returns the subresource index for the specified MIP-map level and array layer.
        UINT CalcSubresource(UINT mipLevel, UINT arrayLayer) const;

        // Returns the subresource index for the specified texture location with respect to the type of this texture (i.e. whether or not array layers are included).
        UINT CalcSubresource(const TextureLocation& location) const;

        // Returns the texture region for the specified offset and extent with respect to the type of this texture (i.e. whether or not array layers are handled by the subresource index).
        D3D11_BOX CalcRegion(const Offset3D& offset, const Extent3D& extent) const;

        // Returns the DXGI format of the texture's base format, i.e. GetBaseFormat() converted to DXGI format.
        DXGI_FORMAT GetBaseDXFormat() const;

        /* ----- Hardware texture objects ----- */

        // Returns the native D3D texture object as <ID3D11Resource*>.
        inline ID3D11Resource* GetNative() const
        {
            return native_.Get();
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

        // Returns the base texture format. Equivalent of GetFormat().
        inline Format GetBaseFormat() const
        {
            return baseFormat_;
        }

        // Returns the DXGI format of the texture object. This can also be a typeless format, i.e. DXGI_FORMAT_*_TYPELESS.
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

        // Returnst the binding table locator for this object.
        inline D3D11BindingLocator* GetBindingLocator()
        {
            return &bindingLocator_;
        }

    private:

        void CreateTexture1D(
            ID3D11Device*                   device,
            const TextureDescriptor&        desc,
            const D3D11_SUBRESOURCE_DATA*   initialData = nullptr
        );

        void CreateTexture2D(
            ID3D11Device*                   device,
            const TextureDescriptor&        desc,
            const D3D11_SUBRESOURCE_DATA*   initialData = nullptr
        );

        void CreateTexture3D(
            ID3D11Device*                   device,
            const TextureDescriptor&        desc,
            const D3D11_SUBRESOURCE_DATA*   initialData = nullptr
        );

        void CreateDefaultResourceViews(ID3D11Device* device, long bindFlags);
        void CreateDefaultSRV(ID3D11Device* device);
        void CreateDefaultUAV(ID3D11Device* device);

        void SetResourceParams(DXGI_FORMAT format, const Extent3D& extent, UINT mipLevels, UINT arraySize);

    private:

        ComPtr<ID3D11Resource>              native_;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

        Format                              baseFormat_         = Format::Undefined;
        DXGI_FORMAT                         format_             = DXGI_FORMAT_UNKNOWN;
        UINT                                numMipLevels_       = 0;
        UINT                                numArrayLayers_     = 0;

        D3D11BindingLocator                 bindingLocator_;

};


} // /namespace LLGL


#endif



// ================================================================================
