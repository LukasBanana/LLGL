/*
 * Image.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_IMAGE_H__
#define __LLGL_IMAGE_H__


#include "Export.h"
#include "RenderSystemFlags.h"
#include "TextureFlags.h"
#include <memory>
#include <vector>//!!!


namespace LLGL
{


/* ----- Types ----- */

/**
\brief Image buffer type.
\remarks Commonly this would be an std::vector<char>, but the image conversion is an optimized process,
where the default initialization of an std::vector is undesired.
Therefore, the image buffer type is an std::unique_ptr<char[]>.
\see ConvertImage
*/
using ImageBuffer = std::unique_ptr<char[]>;


/* ----- Enumerations ----- */

//! Renderer data types enumeration.
enum class DataType
{
    Int8,   //!< 8-bit signed integer (char).
    UInt8,  //!< 8-bit unsigned integer (unsigned char).

    Int16,  //!< 16-bit signed integer (short).
    UInt16, //!< 16-bit unsigned integer (unsigned short).

    Int32,  //!< 32-bit signed integer (int).
    UInt32, //!< 32-bit unsigned integer (unsiged int).
    
    Float,  //!< 32-bit floating-point (float).
    Double, //!< 64-bit real type (double).
};

//! Image format used to write texture data.
enum class ImageFormat
{
    R,              //!< Single color component: Red.
    RG,             //!< Two color components: Red, Green.
    RGB,            //!< Three color components: Red, Green, Blue.
    BGR,            //!< Three color components: Blue, Green, Red.
    RGBA,           //!< Four color components: Red, Green, Blue, Alpha.
    BGRA,           //!< Four color components: Blue, Green, Red, Alpha.
    Depth,          //!< Single color component used as depth component.
    DepthStencil,   //!< Pair of depth and stencil component.
    CompressedRGB,  //!< Generic compressed format with three color components: Red, Green, Blue.
    CompressedRGBA, //!< Generic compressed format with four color components: Red, Green, Blue, Alpha.
};


/* ----- Structures ----- */

/**
\brief Image descriptor structure.
\remarks This kind of 'Image' is mainly used to fill the image data of a hardware texture.
*/
struct LLGL_EXPORT ImageDescriptor
{
    ImageDescriptor() = default;

    // Constructor for uncompressed image data.
    ImageDescriptor(ImageFormat format, DataType dataType, const void* buffer) :
        format  ( format   ),
        dataType( dataType ),
        buffer  ( buffer   )
    {
    }

    //! Constructor for compressed image data.
    ImageDescriptor(ImageFormat format, const void* buffer, unsigned int compressedSize) :
        format          ( format         ),
        buffer          ( buffer         ),
        compressedSize  ( compressedSize )
    {
    }

    ImageFormat     format          = ImageFormat::RGBA;    //!< Specifies the image format. By default ImageFormat::RGBA.
    DataType        dataType        = DataType::UInt8;      //!< Specifies the image data type. This must be DataType::UInt8 for compressed images.
    const void*     buffer          = nullptr;              //!< Pointer to the image buffer.
    unsigned int    compressedSize  = 0;                    //!< Specifies the size (in bytes) of a compressed image. This must be 0 for uncompressed images.
};


/* ----- Functions ----- */

//! Returns the size (in bytes) of the specified data type.
LLGL_EXPORT std::size_t DataTypeSize(const DataType dataType);

/**
\brief Returns the size (in number of components) of the specified image format.
\param[in] imageFormat Specifies the image format.
\return Number of components of the specified image format, or 0 if 'imageFormat' specifies a compressed color format.
\see IsCompressedFormat(const ImageFormat)
*/
LLGL_EXPORT std::size_t ImageFormatSize(const ImageFormat imageFormat);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format);

/**
\brief Returns true if the specified color foramt is a depth-stencil format,
i.e. either ImageFormat::Depth or ImageFormat::DepthStencil.
*/
LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat format);

/**
\brief Converts the image format and data type of the source image (only uncompressed color formats).
\param[in] srcFormat Specifies the source image format.
\param[in] srcDataType Specifies the source data type.
\param[in] srcBuffer Pointer to the source image buffer which is to be converted.
\param[in] srcBufferSize Specifies the size (in bytes) of the source image buffer.
\param[in] dstFormat Specifies the destination image format.
\param[in] dstDataType Specifies the destination data type.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is 0, no multi-threading is used. By default 0.
\return Image buffer with the converted image data or null if no conversion is necessary.
This can be casted to the respective target data type (e.g. "unsigned char", "int", "float" etc.).
\remarks Compressed images and depth-stencil images can not be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination,
if a depth-stencil format is specified either as source or destination,
if the source buffer size is not a multiple of the source data type size times the image format size,
or if 'srcBuffer' is a null pointer.
\see ImageBuffer
\see DataTypeSize
*/
LLGL_EXPORT ImageBuffer ConvertImageBuffer(
    ImageFormat srcFormat,
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    ImageFormat dstFormat,
    DataType    dstDataType,
    std::size_t threadCount = 0
);


#if 1 //TODO: remove this class

/**
\brief Helper class to convert image buffer formats.
\remarks This is mainly used by the renderer, especially by the "SetupTexture..." functions
when the input data must be converted before it can be uploaded to the GPU.
For each conversion function 'srcImage' is the pointer to the source image buffer which is to be converted
and 'imageSize' specifies the size (in bytes) of this source image buffer.
If 'srcImage' is null, an empty image buffer with the respective size is returned.
*/
class LLGL_EXPORT ImageConverter
{

    public:

        static std::vector<char> RGBtoRGBA_Int8(const char* srcImage, std::size_t imageSize);
        static std::vector<unsigned char> RGBtoRGBA_UInt8(const unsigned char* srcImage, std::size_t imageSize);

        static std::vector<short> RGBtoRGBA_Int16(const short* srcImage, std::size_t imageSize);
        static std::vector<unsigned short> RGBtoRGBA_UInt16(const unsigned short* srcImage, std::size_t imageSize);

        /**
        \brief Converts the specified 64-bit double precision image into a 32-bit single precision image.
        \param[in] srcImage Pointer to the source image buffer which is to be converted.
        \param[in] imageSize Specifies the size (in bytes) of the source image buffer.
        */
        static std::vector<float> Float64toFloat32(const double* srcImage, std::size_t imageSize);

};

#endif


} // /namespace LLGL


#endif



// ================================================================================
