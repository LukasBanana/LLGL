/*
 * TestImageConversions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Utils/Image.h>
#include <LLGL/Utils/TypeNames.h>
#include <thread>


DEF_RITEST( ImageConversions )
{
    const std::string imagePath = "../Media/Textures/";
    const std::string refPath   = "Reference/";
    const std::string outputDir = opt.outputDir;

    auto MakeOutputFilename = [](const std::string& filename, ImageFormat format, unsigned threadCount) -> std::string
    {
        std::string s = filename.substr(0, filename.find_last_of('.'));
        s += '-';
        s += ToString(format);
        s += '-';
        s += (threadCount == LLGL_MAX_THREAD_COUNT ? std::string("Max") : std::to_string(threadCount));
        s += ".png";
        return s;
    };

    auto TestConversion = [&](const std::string& filename, unsigned threadCount, double& outTime) -> TestResult
    {
        Image img = TestbedContext::LoadImageFromFile(imagePath + filename);

        const Image imgCopy = img;

        const unsigned w = static_cast<unsigned>(img.GetExtent().width);
        const unsigned h = static_cast<unsigned>(img.GetExtent().height);

        std::uint64_t totalTime = 0;

        #define CONVERT_IMAGE(FORMAT, TYPE, SIZE)                                                           \
            {                                                                                               \
                const std::uint64_t startTime = Timer::Tick();                                              \
                img.Convert((FORMAT), (TYPE), threadCount);                                                 \
                const std::uint64_t endTime = Timer::Tick();                                                \
                totalTime += (endTime - startTime);                                                         \
                if (img.GetDataSize() != (SIZE))                                                            \
                {                                                                                           \
                    const std::string name = MakeOutputFilename(filename, img.GetFormat(), threadCount);    \
                    Log::Errorf(                                                                            \
                        "Mismatch between image size '%s' (%u bytes) and expected size (%u bytes)\n",       \
                        name.c_str(), static_cast<unsigned>(img.GetDataSize()), static_cast<unsigned>(SIZE) \
                    );                                                                                      \
                    return TestResult::FailedMismatch;                                                      \
                }                                                                                           \
            }

        // Convert image through chain of format and data type transitions
        CONVERT_IMAGE(ImageFormat::BGR, DataType::Float16, w*h*3*2);
        CONVERT_IMAGE(ImageFormat::ABGR, DataType::Float16, w*h*4*2);
        CONVERT_IMAGE(ImageFormat::ARGB, DataType::Float64, w*h*4*8);
        CONVERT_IMAGE(ImageFormat::RGB, DataType::UInt8, w*h*3*1);
        TestbedContext::SaveImageToFile(img, outputDir + MakeOutputFilename(filename, img.GetFormat(), threadCount));

        CONVERT_IMAGE(ImageFormat::BGR, DataType::UInt8, w*h*3*1);
        TestbedContext::SaveImageToFile(img, outputDir + MakeOutputFilename(filename, img.GetFormat(), threadCount));

        CONVERT_IMAGE(ImageFormat::R, DataType::UInt8, w*h*1*1);
        TestbedContext::SaveImageToFile(img, outputDir + MakeOutputFilename(filename, img.GetFormat(), threadCount));

        // Store total timing (in seconds)
        outTime = static_cast<double>(totalTime) / static_cast<double>(Timer::Frequency());

        // Evaluate final image data: the red channels must be the same (with a threshold due to floating point conversions) as the original image
        std::uint8_t finalPixel[3];
        MutableImageView finalPixelView;
        finalPixelView.format   = ImageFormat::RGB;
        finalPixelView.dataType = DataType::UInt8;
        finalPixelView.data     = finalPixel;
        finalPixelView.dataSize = sizeof(finalPixel);

        std::uint8_t origPixel[1];
        MutableImageView origPixelView;
        origPixelView.format    = ImageFormat::R;
        origPixelView.dataType  = DataType::UInt8;
        origPixelView.data      = origPixel;
        origPixelView.dataSize  = sizeof(origPixel);

        const int tolerance = 1;

        for (unsigned y = 0; y < img.GetExtent().height; ++y)
        {
            for (unsigned x = 0; x < img.GetExtent().width; ++x)
            {
                const Offset3D readPos  = { static_cast<std::int32_t>(x), static_cast<std::int32_t>(y), 0 };
                const Extent3D readSize = { 1, 1, 1 };

                // Read final and original pixels
                img.ReadPixels(readPos, readSize, finalPixelView);
                imgCopy.ReadPixels(readPos, readSize, origPixelView);

                // Compare the red channels
                const int p0 = static_cast<int>(finalPixel[0]);
                const int p1 = static_cast<int>(origPixel[0]);
                if (std::abs(p0 - p1) > tolerance)
                {
                    Log::Errorf(
                        "Mismatch between final pixel [%u,%u] of image '%s' (R=%d) and original pixel (R=%d)\n",
                        x, y, filename.c_str(), p0, p1
                    );
                    return TestResult::FailedMismatch;
                }
            }
        }

        return TestResult::Passed;
    };

    #define TEST_CONVERSION_THREADED(FILENAME, THREADS, TIME)                                           \
        {                                                                                               \
            TestResult result = TestConversion((FILENAME), (THREADS), (TIME));                          \
            if (result != TestResult::Passed)                                                           \
            {                                                                                           \
                Log::Errorf("ImageConversion(\"%s\", threads: %d) failed\n", (FILENAME), (THREADS));    \
                return result;                                                                          \
            }                                                                                           \
        }

    #define TEST_CONVERSION(FILENAME)                                                                               \
        {                                                                                                           \
            double totalTimes[3] = { 0.0, 0.0, 0.0 };                                                               \
            TEST_CONVERSION_THREADED((FILENAME), 0, totalTimes[0]);                                                 \
            TEST_CONVERSION_THREADED((FILENAME), 2, totalTimes[1]);                                                 \
            TEST_CONVERSION_THREADED((FILENAME), LLGL_MAX_THREAD_COUNT, totalTimes[2]);                             \
            if (opt.showTiming)                                                                                     \
            {                                                                                                       \
                const unsigned maxThreads = std::max(3u, std::thread::hardware_concurrency());                      \
                Log::Printf(                                                                                        \
                    "Conversions for '%s': 1 Thread (%.4f ms), 2 Threads (%.4f ms), %u Threads (%.4f ms):\n",       \
                    (FILENAME), totalTimes[0] * 1000.0, totalTimes[1] * 1000.0, maxThreads, totalTimes[2] * 1000.0  \
                );                                                                                                  \
            }                                                                                                       \
        }

    TEST_CONVERSION("Gradient.png");

    if (!opt.fastTest)
    {
        TEST_CONVERSION("Grid10x10.png");
        TEST_CONVERSION("JohannesVermeer-girl_with_a_pearl_earring.jpg");
        TEST_CONVERSION("VanGogh-starry_night.jpg");
    }

    return TestResult::Passed;
}


