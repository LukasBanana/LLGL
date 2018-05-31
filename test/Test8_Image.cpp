/*
 * Test8_Image.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Image.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../tutorial/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../tutorial/stb_image_write.h"


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

    img1.ReadPixels(LLGL::Offset3D { 0, 0, 0 }, img1Sub.GetExtent(), img1Sub.QueryDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-a.png");

    img1.ReadPixels(LLGL::Offset3D { 109, 0, 0 }, img1Sub.GetExtent(), img1Sub.QueryDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-b.png");

    img1.ReadPixels(LLGL::Offset3D { 328, 164, 0 }, img1Sub.GetExtent(), img1Sub.QueryDstDesc());
    SaveImagePNG(img1Sub, "Output/img1Sub-c.png");

    #if 0
    img1.WritePixels(LLGL::Offset3D { 0, 110, 0 }, img1Sub.GetExtent(), img1Sub.QuerySrcDesc());
    SaveImagePNG(img1, "Output/img1.png");
    #elif 1
    img1.WritePixels(LLGL::Offset3D { 0, 220, 0 }, LLGL::Extent3D { img1.GetExtent().width, 110, 1 }, img1.QuerySrcDesc());
    SaveImagePNG(img1, "Output/img1-write.png");
    #endif
}

void Test_Blit()
{
    auto img1 = LoadImage("Media/Textures/Grid.png", LLGL::ImageFormat::RGBA);

    img1.Blit(LLGL::Offset3D { -27, 0, 0 }, img1, LLGL::Offset3D { 383, 383, 0 }, LLGL::Extent3D { 54, 55, 1 });

    SaveImagePNG(img1, "Output/img1-blit.png");
}

int main(int argc, char* argv[])
{
    try
    {
        Test_PixelOperations();
        Test_Blit();
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
