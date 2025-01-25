/*
 * Format.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Format.h>
#include <tuple>


namespace LLGL
{


namespace Internal
{


// Shortcuts for the format flags
static constexpr long Depth         = FormatFlags::HasDepth;
static constexpr long sRGB          = FormatFlags::IsColorSpace_sRGB;
static constexpr long Compr         = FormatFlags::IsCompressed;
static constexpr long Norm          = FormatFlags::IsNormalized;
static constexpr long Integer       = FormatFlags::IsInteger;
static constexpr long Unsigned      = FormatFlags::IsUnsigned;
static constexpr long Packed        = FormatFlags::IsPacked;
static constexpr long RTV           = FormatFlags::SupportsRenderTarget;
static constexpr long Mips          = FormatFlags::SupportsMips;
static constexpr long GenMips       = FormatFlags::SupportsGenerateMips | Mips | RTV;
static constexpr long Dim1D         = FormatFlags::SupportsTexture1D;
static constexpr long Dim2D         = FormatFlags::SupportsTexture2D;
static constexpr long Dim3D         = FormatFlags::SupportsTexture3D;
static constexpr long DimCube       = FormatFlags::SupportsTextureCube;
static constexpr long Vertex        = FormatFlags::SupportsVertex;

static constexpr long Dim1D_2D      = Dim1D | Dim2D;
static constexpr long Dim2D_3D      = Dim2D | Dim3D;
static constexpr long Dim1D_2D_3D   = Dim1D | Dim2D | Dim3D;
static constexpr long SInt          = Integer;
static constexpr long UInt          = Integer | Unsigned;
static constexpr long SNorm         = Norm;
static constexpr long UNorm         = Unsigned | Norm;
static constexpr long SFloat        = 0;
static constexpr long UFloat        = Unsigned;

static constexpr long Stencil       = FormatFlags::HasStencil | UInt;

// Declaration of all hardware format descriptors
static const FormatAttributes g_formatAttribs[] =
{
//   bits  w  h  c  format                     dataType
    {   0, 0, 0, 0, ImageFormat::R,            DataType::Undefined, 0                                                          }, // Undefined

    /* --- Alpha channel color formats --- */
//   bits  w  h  c  format                     dataType
    {   8, 1, 1, 1, ImageFormat::Alpha,        DataType::UInt8,     GenMips | Dim1D_2D_3D | DimCube | UNorm                    }, // A8UNorm

    /* --- Red channel color formats --- */
//   bits  w  h  c  format                     dataType
    {   8, 1, 1, 1, ImageFormat::R,            DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // R8UNorm
    {   8, 1, 1, 1, ImageFormat::R,            DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // R8SNorm
    {   8, 1, 1, 1, ImageFormat::R,            DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // R8UInt
    {   8, 1, 1, 1, ImageFormat::R,            DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // R8SInt

    {  16, 1, 1, 1, ImageFormat::R,            DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // R16UNorm
    {  16, 1, 1, 1, ImageFormat::R,            DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // R16SNorm
    {  16, 1, 1, 1, ImageFormat::R,            DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // R16UInt
    {  16, 1, 1, 1, ImageFormat::R,            DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // R16SInt
    {  16, 1, 1, 1, ImageFormat::R,            DataType::Float16,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // R16Float

    {  32, 1, 1, 1, ImageFormat::R,            DataType::UInt32,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // R32UInt
    {  32, 1, 1, 1, ImageFormat::R,            DataType::Int32,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // R32SInt
    {  32, 1, 1, 1, ImageFormat::R,            DataType::Float32,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // R32Float

    {  64, 1, 1, 1, ImageFormat::R,            DataType::Float64,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // R64Float

    /* --- RG color formats --- */
//   bits  w  h  c  format                     dataType
    {  16, 1, 1, 2, ImageFormat::RG,           DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RG8UNorm
    {  16, 1, 1, 2, ImageFormat::RG,           DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RG8SNorm
    {  16, 1, 1, 2, ImageFormat::RG,           DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RG8UInt
    {  16, 1, 1, 2, ImageFormat::RG,           DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RG8SInt

    {  32, 1, 1, 2, ImageFormat::RG,           DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RG16UNorm
    {  32, 1, 1, 2, ImageFormat::RG,           DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RG16SNorm
    {  32, 1, 1, 2, ImageFormat::RG,           DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RG16UInt
    {  32, 1, 1, 2, ImageFormat::RG,           DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RG16SInt
    {  32, 1, 1, 2, ImageFormat::RG,           DataType::Float16,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RG16Float

    {  64, 1, 1, 2, ImageFormat::RG,           DataType::UInt32,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RG32UInt
    {  64, 1, 1, 2, ImageFormat::RG,           DataType::Int32,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RG32SInt
    {  64, 1, 1, 2, ImageFormat::RG,           DataType::Float32,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RG32Float

    { 128, 1, 1, 2, ImageFormat::RG,           DataType::Float64,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RG64Float

    /* --- RGB color formats --- */
//   bits  w  h  c  format                     dataType
    {  24, 1, 1, 3, ImageFormat::RGB,          DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RGB8UNorm
    {  24, 1, 1, 3, ImageFormat::RGB,          DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm | sRGB    }, // RGB8UNorm_sRGB
    {  24, 1, 1, 3, ImageFormat::RGB,          DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RGB8SNorm
    {  24, 1, 1, 3, ImageFormat::RGB,          DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGB8UInt
    {  24, 1, 1, 3, ImageFormat::RGB,          DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGB8SInt

    {  48, 1, 1, 3, ImageFormat::RGB,          DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RGB16UNorm
    {  48, 1, 1, 3, ImageFormat::RGB,          DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RGB16SNorm
    {  48, 1, 1, 3, ImageFormat::RGB,          DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGB16UInt
    {  48, 1, 1, 3, ImageFormat::RGB,          DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGB16SInt
    {  48, 1, 1, 3, ImageFormat::RGB,          DataType::Float16,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGB16Float

    {  96, 1, 1, 3, ImageFormat::RGB,          DataType::UInt32,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGB32UInt
    {  96, 1, 1, 3, ImageFormat::RGB,          DataType::Int32,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGB32SInt
    {  96, 1, 1, 3, ImageFormat::RGB,          DataType::Float32,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGB32Float

    { 192, 1, 1, 3, ImageFormat::RGB,          DataType::Float64,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGB64Float

    /* --- RGBA color formats --- */
//   bits  w  h  c  format                     dataType
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RGBA8UNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm | sRGB    }, // RGBA8UNorm_sRGB
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RGBA8SNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt8,     Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGBA8UInt
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::Int8,      Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGBA8SInt

    {  64, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UNorm           }, // RGBA16UNorm
    {  64, 1, 1, 4, ImageFormat::RGBA,         DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SNorm           }, // RGBA16SNorm
    {  64, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt16,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGBA16UInt
    {  64, 1, 1, 4, ImageFormat::RGBA,         DataType::Int16,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGBA16SInt
    {  64, 1, 1, 4, ImageFormat::RGBA,         DataType::Float16,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGBA16Float

    { 128, 1, 1, 4, ImageFormat::RGBA,         DataType::UInt32,    Vertex | GenMips | Dim1D_2D_3D | DimCube | UInt            }, // RGBA32UInt
    { 128, 1, 1, 4, ImageFormat::RGBA,         DataType::Int32,     Vertex | GenMips | Dim1D_2D_3D | DimCube | SInt            }, // RGBA32SInt
    { 128, 1, 1, 4, ImageFormat::RGBA,         DataType::Float32,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGBA32Float

    { 256, 1, 1, 4, ImageFormat::RGBA,         DataType::Float64,   Vertex | GenMips | Dim1D_2D_3D | DimCube | SFloat          }, // RGBA64Float

    /* --- BGRA color formats --- */
//   bits  w  h  c  format                     dataType
    {  32, 1, 1, 4, ImageFormat::BGRA,         DataType::UInt8,     GenMips | Dim1D_2D_3D | DimCube | UNorm                    }, // BGRA8UNorm
    {  32, 1, 1, 4, ImageFormat::BGRA,         DataType::UInt8,     GenMips | Dim1D_2D_3D | DimCube | UNorm | sRGB             }, // BGRA8UNorm_sRGB
    {  32, 1, 1, 4, ImageFormat::BGRA,         DataType::Int8,      GenMips | Dim1D_2D_3D | DimCube | SNorm                    }, // BGRA8SNorm
    {  32, 1, 1, 4, ImageFormat::BGRA,         DataType::UInt8,     GenMips | Dim1D_2D_3D | DimCube | UInt                     }, // BGRA8UInt
    {  32, 1, 1, 4, ImageFormat::BGRA,         DataType::Int8,      GenMips | Dim1D_2D_3D | DimCube | SInt                     }, // BGRA8SInt

    /* --- Packed formats --- */
//   bits  w  h  c  format                     dataType
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::Undefined, GenMips | Dim1D_2D_3D | DimCube | UNorm  | Packed          }, // RGB10A2UNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,         DataType::Undefined, GenMips | Dim1D_2D_3D | DimCube | UInt   | Packed          }, // RGB10A2UInt
    {  32, 1, 1, 3, ImageFormat::RGB,          DataType::Undefined, GenMips | Dim1D_2D_3D | DimCube | UFloat | Packed          }, // RG11B10Float
    {  32, 1, 1, 3, ImageFormat::RGB,          DataType::Undefined, Mips    | Dim1D_2D_3D | DimCube | UFloat | Packed          }, // RGB9E5Float

    /* --- Depth-stencil formats --- */
//   bits  w  h  c  format                     dataType
    {  16, 1, 1, 1, ImageFormat::Depth,        DataType::UInt16,    Mips | RTV | Dim1D_2D | DimCube | UNorm  | Depth           }, // D16UNorm
    {  32, 1, 1, 2, ImageFormat::DepthStencil, DataType::UInt32,    Mips | RTV | Dim1D_2D | DimCube | UNorm  | Depth | Stencil }, // D24UNormS8UInt
    {  32, 1, 1, 1, ImageFormat::Depth,        DataType::Float32,   Mips | RTV | Dim1D_2D | DimCube | SFloat | Depth           }, // D32Float
    {  64, 1, 1, 2, ImageFormat::DepthStencil, DataType::Float32,   Mips | RTV | Dim1D_2D | DimCube | SFloat | Depth | Stencil }, // D32FloatS8X24UInt
  //{   8, 1, 1, 1, ImageFormat::Stencil,      DataType::UInt8,     Mips | RTV | Dim1D_2D | DimCube | UInt   | Stencil         }, // S8UInt

    /* --- Block compression (BC) formats --- */
//   bits  w  h  c  format                     dataType
    {  64, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // BC1UNorm
    {  64, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // BC1UNorm_sRGB
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // BC2UNorm
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // BC2UNorm_sRGB
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // BC3UNorm
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // BC3UNorm_sRGB
    {  64, 4, 4, 1, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // BC4UNorm
    {  64, 4, 4, 1, ImageFormat::Compressed,   DataType::Int8,      Mips | Dim2D_3D | DimCube | Compr | SNorm                  }, // BC4SNorm
    { 128, 4, 4, 2, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // BC5UNorm
    { 128, 4, 4, 2, ImageFormat::Compressed,   DataType::Int8,      Mips | Dim2D_3D | DimCube | Compr | SNorm                  }, // BC5SNorm

    /* --- Advanced scalable texture compression (ASTC) formats --- */
//   bits  w  h  c  format                     dataType
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC4x4
    { 128, 4, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC4x4_sRGB
    { 128, 5, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC5x4
    { 128, 5, 4, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC5x4_sRGB
    { 128, 5, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC5x5
    { 128, 5, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC5x5_sRGB
    { 128, 6, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC6x5
    { 128, 6, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC6x5_sRGB
    { 128, 6, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC6x6
    { 128, 6, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC6x6_sRGB
    { 128, 8, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC8x5
    { 128, 8, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC8x5_sRGB
    { 128, 8, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC8x6
    { 128, 8, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC8x6_sRGB
    { 128, 8, 8, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC8x8
    { 128, 8, 8, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC8x8_sRGB
    { 128,10, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC10x5
    { 128,10, 5, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC10x5_sRGB
    { 128,10, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC10x6
    { 128,10, 6, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC10x6_sRGB
    { 128,10, 8, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC10x8
    { 128,10, 8, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC10x8_sRGB
    { 128,10,10, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC10x10
    { 128,10,10, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC10x10_sRGB
    { 128,12,10, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC12x10
    { 128,12,10, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC12x10_sRGB
    { 128,12,12, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ASTC12x12
    { 128,12,12, 4, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ASTC12x12_sRGB

    /* --- Ericsson texture compression (ETC) formats --- */
//   bits  w  h  c  format                     dataType
    {  64, 4, 4, 3, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ETC1UNorm
    {  64, 4, 4, 3, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm                  }, // ETC2UNorm
    {  64, 4, 4, 3, ImageFormat::Compressed,   DataType::UInt8,     Mips | Dim2D_3D | DimCube | Compr | UNorm | sRGB           }, // ETC2UNorm_sRGB
};


} // /namespace Internal


LLGL_EXPORT const FormatAttributes& GetFormatAttribs(const Format format)
{
    auto idx = static_cast<std::size_t>(format);
    if (idx < (sizeof(Internal::g_formatAttribs) / sizeof(Internal::g_formatAttribs[0])))
        return Internal::g_formatAttribs[idx];
    else
        return Internal::g_formatAttribs[0];
}

std::size_t GetMemoryFootprint(const Format format, std::size_t numTexels)
{
    const FormatAttributes& formatAttribs = GetFormatAttribs(format);
    const std::size_t blockSize = formatAttribs.blockWidth * formatAttribs.blockHeight;
    if (blockSize > 0 && numTexels % blockSize == 0)
        return ((numTexels / blockSize * formatAttribs.bitSize) / 8);
    else
        return 0;
}

LLGL_EXPORT std::uint32_t ImageFormatSize(const ImageFormat imageFormat)
{
    switch (imageFormat)
    {
        case ImageFormat::Alpha:        return 1;
        case ImageFormat::R:            return 1;
        case ImageFormat::RG:           return 2;
        case ImageFormat::RGB:          return 3;
        case ImageFormat::BGR:          return 3;
        case ImageFormat::RGBA:         return 4;
        case ImageFormat::BGRA:         return 4;
        case ImageFormat::ARGB:         return 4;
        case ImageFormat::ABGR:         return 4;
        case ImageFormat::Depth:        return 1;
        case ImageFormat::DepthStencil: return 2;
        case ImageFormat::Stencil:      return 1;
        default:                        return 0;
    }
}

// Returns the number of bytes per pixel for the specified image format and data type
static std::uint32_t GetBytesPerPixel(const ImageFormat imageFormat, const DataType dataType)
{
    if (imageFormat == ImageFormat::DepthStencil)
    {
        if (dataType == DataType::UInt32)
            return 4; // 24-bit for depth, 8-bit for stencil
        else if (dataType == DataType::Float32)
            return 8; // 32-bit for depth, 32-bit for stencil
    }
    return (ImageFormatSize(imageFormat) * DataTypeSize(dataType));
}

LLGL_EXPORT std::size_t GetMemoryFootprint(const ImageFormat imageFormat, const DataType dataType, std::size_t numTexels)
{
    return (static_cast<std::size_t>(GetBytesPerPixel(imageFormat, dataType)) * numTexels);
}

LLGL_EXPORT bool IsCompressedFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & FormatFlags::IsCompressed) != 0);
}

LLGL_DEPRECATED_IGNORE_PUSH()

LLGL_EXPORT bool IsCompressedFormat(const ImageFormat imageFormat)
{
    return (imageFormat >= ImageFormat::Compressed && imageFormat <= ImageFormat::BC5);
}

LLGL_DEPRECATED_IGNORE_POP()

LLGL_EXPORT bool IsDepthOrStencilFormat(const Format format)
{
    const auto& formatAttribs = GetFormatAttribs(format);
    return ((formatAttribs.flags & (FormatFlags::HasDepth | FormatFlags::HasStencil)) != 0);
}

LLGL_EXPORT bool IsDepthOrStencilFormat(const ImageFormat imageFormat)
{
    return (imageFormat == ImageFormat::Depth || imageFormat == ImageFormat::DepthStencil || imageFormat == ImageFormat::Stencil);
}

LLGL_EXPORT bool IsDepthAndStencilFormat(const Format format)
{
    const auto& formatAttribs = GetFormatAttribs(format);
    return ((formatAttribs.flags & (FormatFlags::HasDepth | FormatFlags::HasStencil)) == (FormatFlags::HasDepth | FormatFlags::HasStencil));
}

LLGL_EXPORT bool IsDepthFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & (FormatFlags::HasDepth)) != 0);
}

LLGL_EXPORT bool IsStencilFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & FormatFlags::HasStencil) != 0);
}

LLGL_EXPORT bool IsColorFormat(const Format format)
{
    const auto& formatAttribs = GetFormatAttribs(format);
    return (format != Format::Undefined && (formatAttribs.flags & (FormatFlags::HasDepth | FormatFlags::HasStencil)) == 0);
}

LLGL_EXPORT bool IsNormalizedFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & FormatFlags::IsNormalized) != 0);
}

//deprecated
LLGL_EXPORT bool IsIntegralFormat(const Format format)
{
    const auto& formatAttribs = GetFormatAttribs(format);
    return
    (
        (formatAttribs.flags & (FormatFlags::IsInteger | FormatFlags::IsNormalized)) != 0 &&
        (formatAttribs.flags & (FormatFlags::HasDepthStencil | FormatFlags::IsCompressed)) == 0
    );
}

LLGL_EXPORT bool IsIntegerFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & FormatFlags::IsInteger) != 0);
}

LLGL_EXPORT bool IsFloatFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & FormatFlags::IsInteger) == 0);
}

LLGL_EXPORT std::uint32_t DataTypeSize(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Undefined:   break;
        case DataType::Int8:        return 1;
        case DataType::UInt8:       return 1;
        case DataType::Int16:       return 2;
        case DataType::UInt16:      return 2;
        case DataType::Int32:       return 4;
        case DataType::UInt32:      return 4;
        case DataType::Float16:     return 2;
        case DataType::Float32:     return 4;
        case DataType::Float64:     return 8;
    }
    return 0;
}

LLGL_EXPORT bool IsSIntDataType(const DataType dataType)
{
    return (dataType == DataType::Int8 || dataType == DataType::Int16 || dataType == DataType::Int32);
}

LLGL_EXPORT bool IsUIntDataType(const DataType dataType)
{
    return (dataType == DataType::UInt8 || dataType == DataType::UInt16 || dataType == DataType::UInt32);
}

LLGL_EXPORT bool IsFloatDataType(const DataType dataType)
{
    return (dataType >= DataType::Float16 && dataType <= DataType::Float64);
}


} // /namespace LLGL



// ================================================================================
