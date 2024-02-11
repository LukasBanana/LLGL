/*
 * D3D12Texture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_TEXTURE_H
#define LLGL_D3D12_TEXTURE_H


#include <LLGL/Texture.h>
#include "../D3D12Resource.h"
#include <vector>


namespace LLGL
{


class D3D12Buffer;
class D3D12CommandContext;
class D3D12SubresourceContext;

class D3D12Texture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc);

        // Updates the specified subresource, i.e. a single MIP-map level but one or more array layers.
        void UpdateSubresource(
            D3D12SubresourceContext&        context,
            const D3D12_SUBRESOURCE_DATA&   subresourceData,
            const TextureSubresource&       subresource
        );

        // Updates the specified subresource, i.e. a single MIP-map level but one or more array layers.
        void UpdateSubresourceRegion(
            D3D12SubresourceContext&        context,
            const D3D12_SUBRESOURCE_DATA&   subresourceData,
            const TextureRegion&            region
        );

        // Creates a CPU accessible readback buffer for this texture resource.
        void CreateSubresourceCopyAsReadbackBuffer(
            D3D12SubresourceContext&    context,
            const TextureRegion&        region,
            UINT                        plane,
            UINT&                       outRowStride,
            UINT&                       outLayerSize,
            UINT&                       outLayerStride
        );

        // Creates either the default SRV for the entire resource or a subresource.
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc);

        // Creates either the default UAV for the entire resource or a subresource.
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc);

        // Returns the subresource index for the specified MIP-map level and array layer.
        UINT CalcSubresource(UINT mipLevel, UINT arrayLayer, UINT plane = 0) const;

        // Returns the subresource index for the specified texture location with respect to the type of this texture (i.e. whether or not array layers are included).
        UINT CalcSubresource(const TextureLocation& location) const;

        // Returns the native structure for a texture copy location.
        D3D12_TEXTURE_COPY_LOCATION CalcCopyLocation(const TextureLocation& location) const;

        // Returns the native structure for a placed footprint texture copy location of the specified source buffer.
        D3D12_TEXTURE_COPY_LOCATION CalcCopyLocation(ID3D12Resource* srcResource, UINT64 srcOffset, const Extent3D& extent, UINT rowPitch) const;

        // Returns the texture region for the specified offset and extent with respect to the type of this texture (i.e. whether or not array layers are handled by the subresource index).
        D3D12_BOX CalcRegion(const Offset3D& offset, const Extent3D& extent) const;

        // Returns the DXGI format of the texture's base format, i.e. GetBaseFormat() converted to DXGI format.
        DXGI_FORMAT GetBaseDXFormat() const;

        // Returns true if MIP-maps can be generated for this texture .
        bool SupportsGenerateMips() const;

        // Returns the resource wrapper.
        inline D3D12Resource& GetResource()
        {
            return resource_;
        }

        // Returns the constant resource wrapper.
        inline const D3D12Resource& GetResource() const
        {
            return resource_;
        }

        // Returns the native ID3D12Resource object.
        inline ID3D12Resource* GetNative() const
        {
            return resource_.native.Get();
        }

        // Returns the base texture format. Equivalent of GetFormat().
        inline Format GetBaseFormat() const
        {
            return baseFormat_;
        }

        // Returns the hardware resource format.
        inline DXGI_FORMAT GetDXFormat() const
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

        // Returns the extent this texture was created with, i.e. the extent of the base MIP level.
        inline const Extent3D& GetExtent() const
        {
            return extent_;
        }

        // Returns the entire texture subresource.
        inline TextureSubresource GetWholeSubresource() const
        {
            return TextureSubresource{ 0, GetNumArrayLayers(), 0, GetNumMipLevels() };
        }

        // Returns the descriptor heap for the MIP-map chain. Descriptor 0 is SRV of entire MIP-map chain, 1 to N descriptors are for UAVs for MIP-maps 1 to N.
        inline ID3D12DescriptorHeap* GetMipDescHeap() const
        {
            return mipDescHeap_.Get();
        }

    private:

        void CreateNativeTexture(ID3D12Device* device, const TextureDescriptor& desc);

        void CreateShaderResourceViewPrimary(
            ID3D12Device*               device,
            D3D12_SRV_DIMENSION         dimension,
            DXGI_FORMAT                 format,
            UINT                        componentMapping,
            const TextureSubresource&   subresource,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle
        );

        void CreateUnorderedAccessViewPrimary(
            ID3D12Device*               device,
            D3D12_UAV_DIMENSION         dimension,
            DXGI_FORMAT                 format,
            const TextureSubresource&   subresource,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle
        );

        void CreateMipDescHeap(ID3D12Device* device);

    private:

        D3D12Resource                   resource_;

        Format                          baseFormat_     = Format::Undefined;
        DXGI_FORMAT                     format_         = DXGI_FORMAT_UNKNOWN;
        UINT                            numMipLevels_   = 0;
        UINT                            numArrayLayers_ = 0;
        Extent3D                        extent_;

        ComPtr<ID3D12DescriptorHeap>    mipDescHeap_;

};


} // /namespace LLGL


#endif



// ================================================================================
