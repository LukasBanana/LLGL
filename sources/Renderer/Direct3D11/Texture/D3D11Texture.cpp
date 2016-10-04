/*
 * D3D11Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Texture.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D11Texture::D3D11Texture(const TextureType type) :
    Texture( type )
{
}

Gs::Vector3i D3D11Texture::QueryMipLevelSize(int mipLevel) const
{
    Gs::Vector3i size;

    if (hardwareTexture_.resource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        hardwareTexture_.resource->GetType(&dimension);

        UINT level = static_cast<UINT>(mipLevel);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                hardwareTexture_.tex1D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), 1, 1).Cast<int>();
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                hardwareTexture_.tex2D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), (desc.Height << level), 1).Cast<int>();
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                hardwareTexture_.tex3D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), (desc.Height << level), (desc.Depth << level)).Cast<int>();
            }
            break;
        }
    }

    return size;
}

void D3D11Texture::CreateTexture1D(ID3D11Device* device, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    hardwareTexture_.resource.Reset();

    auto hr = device->CreateTexture1D(&desc, initialData, &hardwareTexture_.tex1D);
    DXThrowIfFailed(hr, "failed to create D3D11 1D-texture");

    CreateSRVAndStoreSettings(device, desc.Format, desc.Width, 1, 1);
}

void D3D11Texture::CreateTexture2D(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    hardwareTexture_.resource.Reset();

    auto hr = device->CreateTexture2D(&desc, initialData, &hardwareTexture_.tex2D);
    DXThrowIfFailed(hr, "failed to create D3D11 2D-texture");

    CreateSRVAndStoreSettings(device, desc.Format, desc.Width, desc.Height, 1);
}

void D3D11Texture::CreateTexture3D(ID3D11Device* device, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    hardwareTexture_.resource.Reset();

    auto hr = device->CreateTexture3D(&desc, initialData, &hardwareTexture_.tex3D);
    DXThrowIfFailed(hr, "failed to create D3D11 3D-texture");

    CreateSRVAndStoreSettings(device, desc.Format, desc.Width, desc.Height, desc.Depth);
}

struct TexFormatDesc
{
    ImageFormat format;
    DataType    dataType;
};

static TexFormatDesc GetFormatAndDataType(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_D32_FLOAT:             return { ImageFormat::Depth,            DataType::Float  };
        case DXGI_FORMAT_D24_UNORM_S8_UINT:     return { ImageFormat::DepthStencil,     DataType::Float  };
        case DXGI_FORMAT_R8_UNORM:              return { ImageFormat::R,                DataType::UInt8  };
        case DXGI_FORMAT_R8_SNORM:              return { ImageFormat::R,                DataType::Int8   };
        case DXGI_FORMAT_R16_UNORM:             return { ImageFormat::R,                DataType::UInt16 };
        case DXGI_FORMAT_R16_SNORM:             return { ImageFormat::R,                DataType::Int16  };
        case DXGI_FORMAT_R32_UINT:              return { ImageFormat::R,                DataType::UInt32 };
        case DXGI_FORMAT_R32_SINT:              return { ImageFormat::R,                DataType::Int32  };
        case DXGI_FORMAT_R32_FLOAT:             return { ImageFormat::R,                DataType::Float  };
        case DXGI_FORMAT_R8G8_UNORM:            return { ImageFormat::RG,               DataType::UInt8  };
        case DXGI_FORMAT_R8G8_SNORM:            return { ImageFormat::RG,               DataType::Int8   };
        case DXGI_FORMAT_R16G16_UNORM:          return { ImageFormat::RG,               DataType::UInt16 };
        case DXGI_FORMAT_R16G16_SNORM:          return { ImageFormat::RG,               DataType::Int16  };
        case DXGI_FORMAT_R32G32_UINT:           return { ImageFormat::RG,               DataType::UInt32 };
        case DXGI_FORMAT_R32G32_SINT:           return { ImageFormat::RG,               DataType::Int32  };
        case DXGI_FORMAT_R32G32_FLOAT:          return { ImageFormat::RG,               DataType::Float  };
        case DXGI_FORMAT_R32G32B32_UINT:        return { ImageFormat::RGB,              DataType::UInt32 };
        case DXGI_FORMAT_R32G32B32_SINT:        return { ImageFormat::RGB,              DataType::Int32  };
        case DXGI_FORMAT_R32G32B32_FLOAT:       return { ImageFormat::RGB,              DataType::Float  };
        case DXGI_FORMAT_R8G8B8A8_UNORM:        return { ImageFormat::RGBA,             DataType::UInt8  };
        case DXGI_FORMAT_R8G8B8A8_SNORM:        return { ImageFormat::RGBA,             DataType::Int8   };
        case DXGI_FORMAT_R16G16B16A16_UNORM:    return { ImageFormat::RGBA,             DataType::UInt16 };
        case DXGI_FORMAT_R16G16B16A16_SNORM:    return { ImageFormat::RGBA,             DataType::Int16  };
        case DXGI_FORMAT_R32G32B32A32_UINT:     return { ImageFormat::RGBA,             DataType::UInt32 };
        case DXGI_FORMAT_R32G32B32A32_SINT:     return { ImageFormat::RGBA,             DataType::Int32  };
        case DXGI_FORMAT_R32G32B32A32_FLOAT:    return { ImageFormat::RGBA,             DataType::Float  };
        case DXGI_FORMAT_BC1_UNORM:             return { ImageFormat::CompressedRGB,    DataType::UInt8  };
        case DXGI_FORMAT_BC2_UNORM:             return { ImageFormat::CompressedRGBA,   DataType::UInt8  };
        case DXGI_FORMAT_BC3_UNORM:             return { ImageFormat::CompressedRGBA,   DataType::UInt8  };
        default:                                break;
    }
    throw std::invalid_argument("failed to convert image buffer into hardware texture format");
}

void D3D11Texture::UpdateSubresource(
    ID3D11DeviceContext* context, UINT mipSlice, UINT arraySlice, const D3D11_BOX& dstBox,
    const ImageDescriptor& imageDesc, std::size_t threadCount)
{
    /* Get destination subresource index */
    auto dstSubresource = D3D11CalcSubresource(mipSlice, arraySlice, numMipLevels_);
    auto srcPitch       = DataTypeSize(imageDesc.dataType)*ImageFormatSize(imageDesc.format);

    /* Check if source image must be converted */
    auto dstTexFormat = GetFormatAndDataType(format_);

    if (dstTexFormat.format != imageDesc.format || dstTexFormat.dataType != imageDesc.dataType)
    {
        /* Get source data stride */
        auto srcRowPitch    = (dstBox.right - dstBox.left)*srcPitch;
        auto srcDepthPitch  = (dstBox.bottom - dstBox.top)*srcRowPitch;
        auto imageSize      = (dstBox.back - dstBox.front)*srcDepthPitch;

        /* Convert image data from RGB to RGBA */
        auto tempData = ConvertImageBuffer(
            imageDesc.format, imageDesc.dataType,
            imageDesc.buffer, imageSize,
            dstTexFormat.format, dstTexFormat.dataType,
            threadCount
        );

        /* Get new source data stride */
        srcPitch        = DataTypeSize(dstTexFormat.dataType)*ImageFormatSize(dstTexFormat.format);
        srcRowPitch     = (dstBox.right - dstBox.left)*srcPitch;
        srcDepthPitch   = (dstBox.bottom - dstBox.top)*srcRowPitch;

        /* Update subresource with specified image data */
        context->UpdateSubresource(
            hardwareTexture_.resource.Get(), dstSubresource,
            &dstBox, tempData.get(), srcRowPitch, srcDepthPitch
        );
    }
    else
    {
        /* Get source data stride */
        auto srcRowPitch    = (dstBox.right - dstBox.left)*srcPitch;
        auto srcDepthPitch  = (dstBox.bottom - dstBox.top)*srcRowPitch;

        /* Update subresource with specified image data */
        context->UpdateSubresource(
            hardwareTexture_.resource.Get(), dstSubresource,
            &dstBox, imageDesc.buffer, srcRowPitch, srcDepthPitch
        );
    }
}


/*
 * ====== Private: ======
 */

void D3D11Texture::CreateSRVAndStoreSettings(ID3D11Device* device, DXGI_FORMAT format, UINT width, UINT height, UINT depth)
{
    /* Create SRV for D3D texture */
    auto hr = device->CreateShaderResourceView(hardwareTexture_.resource.Get(), nullptr, &srv_);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resouce-view (SRV) for texture");

    /* Store format and number of MIP-maps */
    format_         = format;
    numMipLevels_   = NumMipLevels(width, height, depth);
}


} // /namespace LLGL



// ================================================================================
