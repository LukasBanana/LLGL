/*
 * DDSImageReader.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DDSImageReader.h"
#include <fstream>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <type_traits>


static const std::uint32_t ddsMagicNumber = 0x20534444;

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

void DDSImageReader::LoadFromFile(const std::string& filename)
{
    // Open file for reading
    std::ifstream file(filename, std::ios::binary);
    if (!file.good())
        throw std::runtime_error("failed to load DDS image from file: " + filename);

    // Read magic number
    if (ReadValue<std::int32_t>(file) != ddsMagicNumber)
        throw std::runtime_error("invalid magic number in DDS image: " + filename);

    // Rerad DDS header
    DDSHeader header;
    ReadValue(file, header);

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
        imageDesc_.format   = LLGL::ImageFormat::BC1;
    }
    else if (IsFourCC("DXT2") || IsFourCC("DXT3"))
    {
        // Read image with BC2 compression (DXT2 or DXT3)
        texDesc_.format     = LLGL::Format::BC2UNorm;
        imageDesc_.format   = LLGL::ImageFormat::BC2;
    }
    else if (IsFourCC("DXT4") || IsFourCC("DXT5"))
    {
        // Read image with BC3 compression (DXT4 or DXT5)
        texDesc_.format     = LLGL::Format::BC3UNorm;
        imageDesc_.format   = LLGL::ImageFormat::BC3;
    }
    /*else if (IsFourCC("DX10"))
    {
        // Read header extension
        ReadValue(file, headerDX10);
        hasDX10Header = true;
    }*/
    else
    {
        // Print error with FourCC as string
        union
        {
            char            str[4];
            std::int32_t    i32;
        }
        forceCC;
        forceCC.i32 = header.format.fourCC;

        std::stringstream err;
        err << "DDS image has unsupported FourCC value: " << forceCC.str[0] << forceCC.str[1] << forceCC.str[2] << forceCC.str[3];
        throw std::runtime_error(err.str());
    }

    // Read image buffer
    LLGL::Extent3D extent = texDesc_.extent;
    std::uint32_t bufferSize = 0;

    for (std::int32_t mipLevel = 0; mipLevel < header.mipMapCount; ++mipLevel)
    {
        bufferSize += extent.width * extent.height * extent.depth;
        extent.width    = std::max(extent.width  / 2, 1u);
        extent.height   = std::max(extent.height / 2, 1u);
        extent.depth    = std::max(extent.depth  / 2, 1u);
    }

    if (texDesc_.format == LLGL::Format::BC1UNorm)
        bufferSize /= 2;

    data_.resize(bufferSize);

    file.read(data_.data(), bufferSize);

    // Store final image source descriptor
    imageDesc_.dataType = LLGL::DataType::UInt8;
    imageDesc_.data     = data_.data();
    imageDesc_.dataSize = data_.size();
}


