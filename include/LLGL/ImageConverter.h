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
#include <vector>


namespace LLGL
{


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


} // /namespace LLGL


#endif



// ================================================================================
