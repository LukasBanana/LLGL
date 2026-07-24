/*
 * TestFormats.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Trap.h>
#include <LLGL/Utils/TypeNames.h>


// This test ensures the Format enumeration and their functions remain in tact.
DEF_RITEST( FormatAttribs )
{
    TestResult result = TestResult::Passed;

    auto TestFormatAttribs = [](Format format, const char* formatName, DataType dataType, std::uint32_t numPixels, std::uint32_t byteSize) -> TestResult
    {
        const FormatAttributes& formatAttribs = GetFormatAttribs(format);
        if (formatAttribs.bitSize == 0)
        {
            Log::Errorf(
                Log::ColorFlags::StdError,
                "LLGL::Format::%s returned undefined attribute (out of bounds)\n",
                ToString(format)
            );
            return TestResult::FailedErrors;
        }
        if (formatAttribs.blockWidth * formatAttribs.blockHeight != numPixels ||
            formatAttribs.bitSize != byteSize * 8u ||
            formatAttribs.dataType != dataType)
        {
            Log::Errorf(
                Log::ColorFlags::StdError,
                "LLGL::Format::%s returned mismatched attributes:\n"
                " -> bitSize=%u, blockWidth=%u, blockHeight=%u, components=%u, format=LLGL::ImageFormat::%s, dataType=0x%08X, flags=0x%08X\n",
                ToString(format),
                static_cast<unsigned>(formatAttribs.bitSize),
                static_cast<unsigned>(formatAttribs.blockWidth),
                static_cast<unsigned>(formatAttribs.blockHeight),
                static_cast<unsigned>(formatAttribs.components),
                ToString(formatAttribs.format),
                static_cast<unsigned>(formatAttribs.dataType),
                static_cast<unsigned>(formatAttribs.flags)
            );
            return TestResult::FailedMismatch;
        }
        LLGL_VERIFY(formatName != nullptr && *formatName != '\0');
        const std::string actualName = std::string("Format::") + ToString(format);
        if (actualName != formatName)
        {
            Log::Errorf(
                Log::ColorFlags::StdError,
                "Mismtach between expected name '%s' and actual name '%s' for format value %08X\n",
                formatName,
                actualName.c_str(),
                static_cast<unsigned>(format)
            );
            return TestResult::FailedMismatch;
        }
        return TestResult::Passed;
    };

    #define TEST_FORMAT_ATTR(FMT, TYPE, PIXELS, BYTES)                                                  \
        {                                                                                               \
            TestResult intermediateResult = TestFormatAttribs((FMT), #FMT, (TYPE), (PIXELS), (BYTES));  \
            if (intermediateResult != TestResult::Passed)                                               \
            {                                                                                           \
                if (opt.greedy)                                                                         \
                    result = intermediateResult;                                                        \
                else                                                                                    \
                    return intermediateResult;                                                          \
            }                                                                                           \
        }

    TEST_FORMAT_ATTR(Format::A8UNorm, DataType::UInt8, 1, 1);
    TEST_FORMAT_ATTR(Format::BGRA8UNorm_sRGB, DataType::UInt8, 1, 4);
    TEST_FORMAT_ATTR(Format::BC1UNorm, DataType::UInt8, 16, 8);
    TEST_FORMAT_ATTR(Format::ASTC4x4, DataType::UInt8, 16, 16);
    TEST_FORMAT_ATTR(Format::ASTC12x12_sRGB, DataType::UInt8, 144, 16);
    TEST_FORMAT_ATTR(Format::ETC2UNorm_sRGB, DataType::UInt8, 16, 8);

    return result;
}


