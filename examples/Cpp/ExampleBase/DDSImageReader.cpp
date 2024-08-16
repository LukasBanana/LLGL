/*
 * DDSImageReader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DDSImageReader.h"
#include "FileUtils.h"
#include <fstream>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <algorithm>
#include <LLGL/Log.h>


static const std::uint32_t ddsMagicNumber = 0x20534444; // 'DDS '

enum DDSFourCCTypes
{
    FOURCC_NONE,
    FOURCC_DXT1, // BC1
    FOURCC_DXT2, // BC2
    FOURCC_DXT3, // BC2
    FOURCC_DXT4, // BC3
    FOURCC_DXT5, // BC3
    FOURCC_DX10,
    FOURCC_BC4U,
    FOURCC_BC4S,
    FOURCC_BC5S,
    FOURCC_ATI2,
    FOURCC_RGBG,
    FOURCC_GRGB,
    FOURCC_UYVY,
    FOURCC_YUY2,
    FOURCC_36,
    FOURCC_110,
    FOURCC_111,
    FOURCC_112,
    FOURCC_113,
    FOURCC_114,
    FOURCC_115,
    FOURCC_116,
    FOURCC_117,
};

enum DDSImageFlags
{
    // Main flags
    DDSFLAG_MIPMAPS     = 0x00020000,
    DDSFLAG_DEPTH       = 0x00800000,

    // Format flags
    DDSFLAG_ALPHA       = 0x00000001,
    DDSFLAG_COMPRESSED  = 0x00000004,

    // Cube map flags
    DDSFLAG_CUBEMAP     = 0x00000200,
};

#if defined _MSC_VER
#   pragma pack(push, packing)
#   pragma pack(1)
#   define PACK_STRUCT
#elif defined __GNUC__ || defined __clang__
#   define PACK_STRUCT __attribute__((packed))
#else
#   define PACK_STRUCT
#endif

struct DDSPixelFormat
{
    std::int32_t    structSize;
    std::int32_t    flags;
    std::int32_t    fourCC; // DDSFourCCTypes
    std::int32_t    rgbBitCount;
    std::int32_t    rBitMask;
    std::int32_t    gBitMask;
    std::int32_t    bBitMask;
    std::int32_t    aBitMask;
}
PACK_STRUCT;

struct DDSHeader
{
    std::int32_t    structSize;
    std::int32_t    flags;
    std::int32_t    height;
    std::int32_t    width;
    std::int32_t    pitch;
    std::int32_t    depth;
    std::int32_t    mipMapCount;
    std::int32_t    reserved1[11];
    DDSPixelFormat  format;
    std::int32_t    surfaceFlags;
    std::int32_t    cubeMapFlags;
    std::int32_t    reserved2[3];
}
PACK_STRUCT;

struct DDSHeaderDX10
{
    std::int32_t    format;
    std::int32_t    dimension;
    std::uint32_t   miscFlag;
    std::uint32_t   arraySize;
    std::uint32_t   reserved;
}
PACK_STRUCT;

#ifdef _MSC_VER
#    pragma pack(pop, packing)
#endif

#undef PACK_STRUCT

// Reads the next value of type <T> from the specified input stream
template <typename T>
void ReadValue(std::istream& stream, T& value)
{
    static_assert(!std::is_pointer<T>::value, "typename T in Read<T> must not be a pointer type");
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
}

// Reads the next value of type <T> from the specified input stream
template <typename T>
T ReadValue(std::istream& stream)
{
    T value;
    ReadValue(stream, value);
    return value;
}

bool DDSImageReader::LoadFromFile(const std::string& filename)
{
    // Open file for reading
    AssetReader reader = ReadAsset(filename);
    if (!reader)
        return false;

    // Read magic number
    if (reader.Read<std::int32_t>() != ddsMagicNumber)
    {
        LLGL::Log::Errorf("invalid magic number in DDS image: %s\n", filename.c_str());
        return false;
    }

    // Read DDS header
    DDSHeader header = reader.Read<DDSHeader>();

    DDSHeaderDX10 headerDX10;
    ::memset(&headerDX10, 0, sizeof(headerDX10));

    //bool hasDX10Header  = false;
    //bool hasMips        = ((header.flags        & DDSFLAG_MIPMAPS   ) != 0);
    bool hasDepth       = ((header.flags        & DDSFLAG_DEPTH     ) != 0);
    //bool hasAlpha       = ((header.format.flags & DDSFLAG_ALPHA     ) != 0);
    //bool isCompressed   = ((header.format.flags & DDSFLAG_COMPRESSED) != 0);
    bool isCubeMap      = ((header.cubeMapFlags & DDSFLAG_CUBEMAP   ) != 0);

    // Store parameters in descriptors
    texDesc_.bindFlags      = LLGL::BindFlags::Sampled;
    texDesc_.miscFlags      = 0;
    texDesc_.extent.width   = static_cast<std::uint32_t>(header.width);
    texDesc_.extent.height  = static_cast<std::uint32_t>(header.height);
    texDesc_.mipLevels      = header.mipMapCount;

    if (isCubeMap)
    {
        texDesc_.type           = LLGL::TextureType::TextureCube;
        texDesc_.extent.depth   = 1;
        texDesc_.arrayLayers    = 6;
    }
    else if (hasDepth)
    {
        texDesc_.type           = LLGL::TextureType::Texture3D;
        texDesc_.extent.depth   = static_cast<std::uint32_t>(header.depth);
        texDesc_.arrayLayers    = 1;
    }
    else
    {
        texDesc_.type           = LLGL::TextureType::Texture2D;
        texDesc_.extent.depth   = 1;
        texDesc_.arrayLayers    = 1;
    }

    // Evaluate ForceCC value
    auto IsFourCC = [&](const char* value)
    {
        return (header.format.fourCC == *reinterpret_cast<const std::int32_t*>(value));
    };

    if (IsFourCC("DXT1"))
    {
        // Read image with BC1 compression (DXT1)
        texDesc_.format     = LLGL::Format::BC1UNorm;
    }
    else if (IsFourCC("DXT2") || IsFourCC("DXT3"))
    {
        // Read image with BC2 compression (DXT2 or DXT3)
        texDesc_.format     = LLGL::Format::BC2UNorm;
    }
    else if (IsFourCC("DXT4") || IsFourCC("DXT5"))
    {
        // Read image with BC3 compression (DXT4 or DXT5)
        texDesc_.format     = LLGL::Format::BC3UNorm;
    }
    /*else if (IsFourCC("DX10"))
    {
        // Read header extension
        (void)reader.Read<headerDX10>();
        hasDX10Header = true;
    }*/
    else
    {
        // Print error with FourCC as string
        char fourCC[5] = {};
        ::memcpy(fourCC, &(header.format.fourCC), sizeof(header.format.fourCC));

        throw std::runtime_error("DDS image has unsupported FourCC value: " + std::string(fourCC));
    }

    // Read image buffer
    LLGL::Extent3D extent = texDesc_.extent;
    std::uint32_t bufferSize = 0;
    MipSection mip = {};

    for (std::int32_t mipLevel = 0; mipLevel < header.mipMapCount; ++mipLevel)
    {
        // Save offset and size of current MIP-map into data container
        mip.offset  = bufferSize;
        mip.size    = extent.width * extent.height * extent.depth;
        if (texDesc_.format == LLGL::Format::BC1UNorm)
            mip.size /= 2;
        mips_.push_back(mip);

        // Reduce extent to keep track of next MIP-map extent
        extent.width    = std::max(extent.width  / 2, 1u);
        extent.height   = std::max(extent.height / 2, 1u);
        extent.depth    = std::max(extent.depth  / 2, 1u);

        // Accumulate buffer size to read data at once
        bufferSize += mip.size;
    }

    data_.resize(bufferSize);

    reader.Read(data_.data(), static_cast<std::streamsize>(data_.size()));

    return true;
}

LLGL::ImageView DDSImageReader::GetImageView(std::uint32_t mipLevel) const
{
    if (mipLevel >= mips_.size())
        return {};
    const MipSection& mip = mips_[mipLevel];
    LLGL::ImageView imageView;
    {
        imageView.format    = LLGL::GetFormatAttribs(texDesc_.format).format;
        imageView.dataType  = LLGL::DataType::UInt8;
        imageView.data      = &(data_[mip.offset]);
        imageView.dataSize  = mip.size;
    }
    return imageView;
}

