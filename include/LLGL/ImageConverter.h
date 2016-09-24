/*
 * ImageConverter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_IMAGE_CONVERTER_H__
#define __LLGL_IMAGE_CONVERTER_H__


#include "Export.h"
#include "RenderSystemFlags.h"
#include "TextureFlags.h"
#include <memory>
#include <vector>//!!!


namespace LLGL
{


/**
\brief Image buffer type.
\remarks Commonly this would be an std::vector<char>, but the image conversion is an optimized process,
where the default initialization of an std::vector is undesired.
Therefore, the image buffer type is an std::unique_ptr<char[]>.
\see ConvertImage
*/
using ImageBuffer = std::unique_ptr<char[]>;

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
