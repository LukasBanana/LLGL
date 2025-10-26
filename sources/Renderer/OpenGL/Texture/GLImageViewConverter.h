/*
 * GLTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_IMAGE_VIEW_CONVERTER_H
#define LLGL_GL_IMAGE_VIEW_CONVERTER_H


#include <LLGL/ImageFlags.h>
#include <LLGL/Container/DynamicArray.h>


namespace LLGL
{


// Predefined texture swizzles to emulate certain texture format
enum class GLSwizzleFormat
{
    RGBA,   // GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA (Identity mapping)
    BGRA,   // GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA
    Alpha,  // GL_ZERO, GL_ZERO, GL_ZERO, GL_RED
};

// Wrapper class to handle GL image data conversion for fomrats with component swizzling.
class GLImageViewConverter
{

    public:

        GLImageViewConverter() = default;

        // Initializes the convert with the specified assigned image.
        GLImageViewConverter(const ImageView* initialImage, GLSwizzleFormat swizzleFormat);

        // Assigns the specified image view with a swizzling format.
        void Assign(const ImageView* initialImage, GLSwizzleFormat swizzleFormat);

        const ImageView* GetView() const
        {
            return imageViewRef_;
        }

    private:

        ImageView           intermediateImageView_;
        DynamicByteArray    intermediateImageBuffer_;
        const ImageView*    imageViewRef_               = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
