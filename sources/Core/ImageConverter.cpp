/*
 * ImageConverter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ImageConverter.h>
#include <limits>
#include <algorithm>


namespace LLGL
{


/* ----- Internal functions ----- */

template <typename T>
void FillEmpty(std::vector<T>& dstImage)
{
    std::fill(dstImage.begin(), dstImage.end(), T(0));
}

template <typename T>
std::vector<T> ConvertImageRGBtoRGBA(const T* srcImage, std::size_t imageSize)
{
    imageSize /= sizeof(T);
    std::vector<T> dstImage(imageSize/3*4);

    if (srcImage)
    {
        for (std::size_t i = 0, j = 0; i < imageSize; i += 3, j += 4)
        {
            dstImage[j  ] = srcImage[i  ];
            dstImage[j+1] = srcImage[i+1];
            dstImage[j+2] = srcImage[i+2];
            dstImage[j+3] = std::numeric_limits<T>::max();
        }
    }
    else
        FillEmpty(dstImage);

    return dstImage;
}

template <typename T0, typename T1>
std::vector<T0> ConvertImageDataType(const T1* srcImage, std::size_t imageSize)
{
    imageSize /= sizeof(T1);
    std::vector<T0> dstImage(imageSize);

    if (srcImage)
    {
        for (std::size_t i = 0; i < imageSize; ++i)
            dstImage[i] = static_cast<T0>(srcImage[i]);
    }
    else
        FillEmpty(dstImage);

    return dstImage;
}


/* ----- Public functions ----- */

LLGL_EXPORT std::vector<char> ConvertImage(
    ImageFormat srcImageFormat,
    DataType    srcDataType,
    const void* srcImageData,
    std::size_t srcImageSize,
    ImageFormat dstImageFormat,
    DataType    dstDataType)
{
    std::vector<char> dstImage;
    
    #if 0//TODO
    if (srcDataType != dstDataType)
    {
        auto dstImageSize = srcImageSize * DataTypeSize(dstDataType) / DataTypeSize(srcDataType);
        dstImage.resize(dstImageSize);

        for (std::size_t i = 0; i < dstImageSize; ++i)
        {
            switch (srcDataType)
            {
                case DataType::Int8:
                case DataType::UInt8:
                case DataType::Int16:
                case DataType::UInt16:
                case DataType::Int32:
                case DataType::UInt32:
                case DataType::Float32:
                case DataType::Float64:
            }
        }
    }
    #endif
    
    return dstImage;
}


#if 1 //TODO: remove this class

std::vector<char> ImageConverter::RGBtoRGBA_Int8(const char* srcImage, std::size_t imageSize)
{
    return ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<unsigned char> ImageConverter::RGBtoRGBA_UInt8(const unsigned char* srcImage, std::size_t imageSize)
{
    return ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<short> ImageConverter::RGBtoRGBA_Int16(const short* srcImage, std::size_t imageSize)
{
    return ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<unsigned short> ImageConverter::RGBtoRGBA_UInt16(const unsigned short* srcImage, std::size_t imageSize)
{
    return ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<float> ImageConverter::Float64toFloat32(const double* srcImage, std::size_t imageSize)
{
    return ConvertImageDataType<float>(srcImage, imageSize);
}

#endif


} // /namespace LLGL



// ================================================================================
