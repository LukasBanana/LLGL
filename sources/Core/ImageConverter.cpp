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
std::vector<T0> ConvertImageType(const T1* srcImage, std::size_t imageSize)
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


/* ----- ImageConverter class ----- */

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
    return ConvertImageType<float>(srcImage, imageSize);
}


} // /namespace LLGL



// ================================================================================
