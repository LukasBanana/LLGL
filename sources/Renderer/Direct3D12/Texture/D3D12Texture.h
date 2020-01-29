/*
 * D3D12Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

class D3D12Texture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

        Format GetFormat() const override;

    public:

        D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc);

        void UpdateSubresource(
            ID3D12Device*               device,
            ID3D12GraphicsCommandList*  commandList,
            ComPtr<ID3D12Resource>&     uploadBuffer,
            D3D12_SUBRESOURCE_DATA&     subresourceData,
            UINT                        mipLevel        = 0,
            UINT                        firstArrayLayer = 0,
            UINT                        numArrayLayers  = ~0
        );

        // Creates a CPU accessible readback buffer for this texture resource.
        void CreateSubresourceCopyAsReadbackBuffer(
            ID3D12Device*           device,
            D3D12CommandContext&    commandContext,
            const TextureRegion&    region,
            ComPtr<ID3D12Resource>& readbackBuffer,
            UINT&                   rowStride
        );

        // Creates either the default SRV for the entire resource or a subresource.
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc);

        // Creates either the default UAV for the entire resource or a subresource.
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle);
        void CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc);

        // Returns the subresource index for the specified MIP-map level and array layer.
        UINT CalcSubresource(UINT mipLevel, UINT arrayLayer) const;

        // Returns the subresource index for the specified texture location with respect to the type of this texture (i.e. whether or not array layers are included).
        UINT CalcSubresource(const TextureLocation& location) const;

        // Returns the native structure for a texture copy location.
        D3D12_TEXTURE_COPY_LOCATION CalcCopyLocation(const TextureLocation& location) const;

        // Returns the native structure for a placed footprint texture copy location of the specified source buffer.
        D3D12_TEXTURE_COPY_LOCATION CalcCopyLocation(const D3D12Buffer& srcBuffer, UINT64 srcOffset, const Extent3D& extent, UINT rowPitch) const;

        // Returns the texture region for the specified offset and extent with respect to the type of this texture (i.e. whether or not array layers are handled by the subresource index).
        D3D12_BOX CalcRegion(const Offset3D& offset, const Extent3D& extent) const;

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

        DXGI_FORMAT                     format_         = DXGI_FORMAT_UNKNOWN;
        UINT                            numMipLevels_   = 0;
        UINT                            numArrayLayers_ = 0;

        ComPtr<ID3D12DescriptorHeap>    mipDescHeap_;

};


} // /namespace LLGL


#endif



// ================================================================================
