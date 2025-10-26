/*
 * GLImageViewConverter.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLImageViewConverter.h"


namespace LLGL
{


GLImageViewConverter::GLImageViewConverter(const ImageView* initialImage, GLSwizzleFormat swizzleFormat)
{
    Assign(initialImage, swizzleFormat);
}

static ImageFormat MapSwizzleImageFormat(const ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::RGBA: return ImageFormat::BGRA;
        case ImageFormat::RGB:  return ImageFormat::BGR;
        default:                return format;
    }
}

void GLImageViewConverter::Assign(const ImageView* initialImage, GLSwizzleFormat swizzleFormat)
{
    imageViewRef_ = initialImage;

    if (initialImage != nullptr)
    {
        /* Re-map image format if texture format must be emulated with component swizzling */
        switch (swizzleFormat)
        {
            case GLSwizzleFormat::RGBA:
            {
                /* Nothign to convert, just use input view */
            }
            break;

            case GLSwizzleFormat::BGRA:
            {
                /* Just convert image format, but keep image buffer unchanged */
                intermediateImageView_ = *initialImage;
                {
                    intermediateImageView_.format = MapSwizzleImageFormat(initialImage->format);
                }
                imageViewRef_ = &intermediateImageView_;
            }
            break;

            case GLSwizzleFormat::Alpha:
            {
                /*
                Only convert image data if input image has RGBA.
                If input is RGB, no alpha can be provided, but if input is Alpha, then there's no reason to convert anyway.
                */
                if (initialImage->format == ImageFormat::RGBA)
                {
                    intermediateImageBuffer_ = ConvertImageBuffer(*initialImage, ImageFormat::Alpha, initialImage->dataType, LLGL_MAX_THREAD_COUNT);
                    {
                        intermediateImageView_.format   = ImageFormat::Alpha;
                        intermediateImageView_.dataType = initialImage->dataType;
                        intermediateImageView_.data     = intermediateImageBuffer_.get();
                        intermediateImageView_.dataSize = initialImage->dataSize / ImageFormatSize(initialImage->format);
                    }
                    imageViewRef_ = &intermediateImageView_;
                }
            }
            break;
        }
    }
}


} // /namespace LLGL



// ================================================================================
