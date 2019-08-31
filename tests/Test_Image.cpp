/*
 * Test_Image.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Image.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


LLGL::Image LoadImage(const std::string& filename, const LLGL::ImageFormat format = LLGL::ImageFormat::RGB)
{
    int requiredComp = static_cast<int>(LLGL::ImageFormatSize(format));

    int w = 0, h = 0, comp = 0;
    auto buf = stbi_load(filename.c_str(), &w, &h, &comp, requiredComp);

    LLGL::Image img
    {
        LLGL::Extent3D { static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h), 1 },
        format,
        LLGL::DataType::UInt8
    };

    ::memcpy(img.GetData(), buf, img.GetDataSize());

    stbi_image_free(buf);

    return img;
}

void SaveImagePNG(const LLGL::Image& img, const std::string& filename, std::uint32_t slice = 0)
{
    int  w    = static_cast<int>(img.GetExtent().width);
    int  h    = static_cast<int>(img.GetExtent().height);
    int  comp = static_cast<int>(LLGL::ImageFormatSize(img.GetFormat()));
    auto buf  = reinterpret_cast<const char*>(img.GetData()) + img.GetDepthStride() * slice;

    stbi_write_png(filename.c_str(), w, h, comp, buf, img.GetRowStride());
}

void Test_PixelOperations()
{
    auto img1 = LoadImage("Media/Textures/Grid.png", LLGL::ImageFormat::RGBA);

    LLGL::Image img1Sub { LLGL::Extent3D { 109, 110, 1 }, LLGL::ImageFormat::BGR, img1.GetDataType() };

    img1.ReadPixels(LLGL::Offset3D { 0, 0, 0 }, img1Sub.GetExtent(), img1Sub.GetDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-a.png");

    img1.ReadPixels(LLGL::Offset3D { 109, 0, 0 }, img1Sub.GetExtent(), img1Sub.GetDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-b.png");

    img1.ReadPixels(LLGL::Offset3D { 328, 164, 0 }, img1Sub.GetExtent(), img1Sub.GetDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-c.png");

    #if 0
    img1.WritePixels(LLGL::Offset3D { 0, 110, 0 }, img1Sub.GetExtent(), img1Sub.GetSrcDesc());
    SaveImagePNG(img1, "Output/img1.png");
    #elif 1
    img1.WritePixels(LLGL::Offset3D { 0, 220, 0 }, LLGL::Extent3D { img1.GetExtent().width, 110, 1 }, img1.GetSrcDesc());
    SaveImagePNG(img1, "Output/img1-write.png");
    #endif
}

void Test_Blit()
{
    auto img1 = LoadImage("Media/Textures/Grid.png", LLGL::ImageFormat::RGBA);

    img1.Blit(LLGL::Offset3D { -27, 0, 0 }, img1, LLGL::Offset3D { 383, 383, 0 }, LLGL::Extent3D { 54, 55, 1 });

    SaveImagePNG(img1, "Output/img1-blit.png");
}

void Test_Resize()
{
    auto img1 = LoadImage("Media/Textures/Grid.png", LLGL::ImageFormat::RGB);

    const auto& extent = img1.GetExtent();

    #if 1
    img1.Resize(
        LLGL::Extent3D { 512, 512, 1 },
        LLGL::ColorRGBAd { 0.0, 1.0, 0.0 },
        LLGL::Offset3D { (512 - static_cast<int>(extent.width))/2, (512 - static_cast<int>(extent.height)) / 2, 0 }
    );
    #else
    img1.Resize(
        img1.GetExtent() + LLGL::Extent3D{ 6, 6, 0 },
        LLGL::ColorRGBAd { 1.0, 0.0, 1.0 },
        LLGL::Offset3D { 3, 3, 0 }
    );
    #endif
    SaveImagePNG(img1, "Output/img1-resize-larger.png");

    img1.Resize(
        LLGL::Extent3D { 128, 128, 1 },
        LLGL::ColorRGBAd { 0.0, 1.0, 0.0 },
        LLGL::Offset3D { (128 - static_cast<int>(extent.width)) / 2, (128 - static_cast<int>(extent.height)) / 2, 0u }
    );
    SaveImagePNG(img1, "Output/img1-resize-smaller.png");
}

int main(int argc, char* argv[])
{
    try
    {
        //Test_PixelOperations();
        //Test_Blit();
        Test_Resize();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }
    return 0;
}
