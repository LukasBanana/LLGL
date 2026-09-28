/*
 * BrickPlay.Utils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_UTILS_H
#define LLGL_EXAMPLE_BRICKPLAY_UTILS_H


#include <LLGL/Utils/ColorRGBA.h>
#include <string>
#include <cstdint>
#include <algorithm>


namespace BrickUtils
{

    inline std::uint32_t ColorToUint32(LLGL::ColorRGBAub col)
    {
        return
        (
            std::uint32_t(col.r) << 24 |
            std::uint32_t(col.g) << 16 |
            std::uint32_t(col.b) <<  8 |
            std::uint32_t(col.a)
        );
    };

    inline LLGL::ColorRGBAub Uint32ToColor(std::uint32_t col)
    {
        return LLGL::ColorRGBAub
        {
             col >> 24        ,
            (col >> 16) & 0xFF,
            (col >>  8) & 0xFF,
            (col      ) & 0xFF
        };
    };

    inline LLGL::ColorRGBAub ColorFromString(const std::string& s)
    {
        LLGL::ColorRGBAub color;

        auto HexToUbyte = [](const char* s) -> std::uint8_t
        {
            std::uint8_t val = 0;
            for (int i = 0; i < 2; ++i)
            {
                val *= 0x10;
                     if (s[i] >= '0' && s[i] <= '9') { val += s[i] - '0';       }
                else if (s[i] >= 'A' && s[i] <= 'F') { val += s[i] - 'A' + 0xA; }
                else if (s[i] >= 'a' && s[i] <= 'f') { val += s[i] - 'a' + 0xA; }
            }
            return val;
        };

        if (s.size() == 2)
        {
            // Parse string as gray-scaled color value
            color.r = color.g = color.b = HexToUbyte(&s[0]);
        }
        else if (s.size() >= 6)
        {
            // Parse RGB values and optional Alpha channel
            color.r = HexToUbyte(&s[0]);
            color.g = HexToUbyte(&s[2]);
            color.b = HexToUbyte(&s[4]);
            if (s.size() == 8)
                color.a = HexToUbyte(&s[6]);
        }

        return color;
    }

    inline const char* ColorToString(LLGL::ColorRGBAub color)
    {
        static thread_local char colorStr[9] = {};

        auto UbyteToHex = [](char* out, std::uint8_t val) -> void
        {
            const char* alphabet = "0123456789ABCDEF";
            out[0] = alphabet[val / 0x10];
            out[1] = alphabet[val % 0x10];
        };

        if (color.r == color.g && color.g == color.b && color.a == 0xFF)
        {
            // Write color out as gray-scaled value
            UbyteToHex(colorStr, color.r);
            colorStr[2] = '\0';
        }
        else
        {
            // Write out RGB channels
            UbyteToHex(colorStr    , color.r);
            UbyteToHex(colorStr + 2, color.g);
            UbyteToHex(colorStr + 4, color.b);

            // Skip alpha channel if its opaque (0xFF)
            if (color.a == 0xFF)
                colorStr[6] = '\0';
            else
                UbyteToHex(colorStr + 6, color.a);
        }

        return colorStr;
    }

    inline bool ParseTextFile(std::istream& file, const std::function<bool(std::stringstream& stream, int lineNo)>& parseLine)
    {
        // Parse every line and interpret it
        int lineNo = 0;
        for (std::string line; std::getline(file, line);)
        {
            ++lineNo;

            // Ignore empty line
            if (line.empty())
                continue;

            // Ignore line only consisting of whitespace characters
            std::size_t firstNoneWhitespace = line.find_first_not_of(" \t");
            if (firstNoneWhitespace == std::string::npos)
                continue;

            // Ignore comment line
            if (line[firstNoneWhitespace] == '#')
                continue;

            // Parse current line with stringstream
            std::stringstream stream{ line };

            if (!parseLine(stream, lineNo))
                return false;
        }

        return true;
    }

    // Returns the specified number as en-US localized string, e.g. 1000 becomes "1,000".
    inline std::string NumToStr_enUS(std::uint64_t n, int base = 10, char sep = ',')
    {
        LLGL_VERIFY(base >= 2 && base <= 16);
        const int cycles[15] = { 8, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 0, 2 };
        const char* alphabet = "0123456789ABCDEF";

        if (n == 0)
            return "0";

        std::string s;

        // Build string in reverse to simplify string allocations
        const int cycle = cycles[base - 2];
        for (int i = 0; n > 0; ++i)
        {
            if (i > 0 && i % cycle == 0)
                s.push_back(sep);
            s.push_back(alphabet[n % base]);
            n /= base;
        }

        // Now flip string from left to right
        for (std::size_t i = 0, sHalfLen = s.size()/2; i < sHalfLen; ++i)
            std::swap(s[i], s[s.size() - 1 - i]);

        return s;
    }

    inline std::string NumToBytes_enUS(std::uint64_t n)
    {
        constexpr std::uint64_t KiB1 = 1024;
        constexpr std::uint64_t MiB2 = KiB1*KiB1;

        if (n == 1)
            return "1 byte";
        if (n > MiB2)
            return NumToStr_enUS(n / MiB2) + '.' + std::to_string((n % MiB2) * 1000 * 1000 / MiB2) + " MiB";
        if (n > KiB1)
            return NumToStr_enUS(n / KiB1) + '.' + std::to_string((n % KiB1) * 1000 / KiB1) + " KiB";

        return NumToStr_enUS(n) + " bytes";
    }

    // Color palette singleton.
    class ColorPalette
    {

        ColorPalette() = default;

        ColorPalette(const ColorPalette&) = delete;
        ColorPalette& operator = (const ColorPalette&) = delete;

        std::vector<LLGL::ColorRGBAub>  colors;
        int                             firstTransparentIndex   = 0xFF;

    public:

        static constexpr int            maxNumColors            = 32;

        static ColorPalette& Get()
        {
            static ColorPalette instance;
            return instance;
        }

        // Create a default color palette
        void InitDefault()
        {
            colors =
            {
                LLGL::ColorRGBAub{ 0xFF, 0xFF, 0xFF }, // White
                LLGL::ColorRGBAub{ 0x85, 0x80, 0x85 }, // Mid gray
                LLGL::ColorRGBAub{ 0x20, 0x25, 0x22 }, // Dark gray
                LLGL::ColorRGBAub{ 0xFF, 0xF0, 0x3A }, // Bright light yellow
                LLGL::ColorRGBAub{ 0x73, 0xDC, 0xA1 }, // Medium green
                LLGL::ColorRGBAub{ 0x62, 0xFA, 0x78 }, // Bright green
                LLGL::ColorRGBAub{ 0x20, 0x32, 0xB0 }, // Dark blue
                LLGL::ColorRGBAub{ 0xFF, 0x69, 0x8F }, // Coral
                LLGL::ColorRGBAub{ 0x92, 0x39, 0x78 }, // Magenta
                LLGL::ColorRGBAub{ 0xD5, 0x40, 0x40 }, // Bright red

                LLGL::ColorRGBAub{ 0xFF, 0xFF, 0xFF, 0xB4 }, // Glass
            };
        }

        // Returns the color palette index for the specified color or -1 if not found.
        int FindIndex(LLGL::ColorRGBAub inColor) const
        {
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                if (colors[i] == inColor)
                    return static_cast<int>(i);
            }
            return -1;
        }

        int FindClosestColorIndex(LLGL::ColorRGBAub inColor) const
        {
            auto ColorToRank = [](LLGL::ColorRGBAub col) -> int
            {
                return col.a * 255*3 + (col.r + col.g + col.b);
            };

            const int inColorRank = ColorToRank(inColor);
            int prevRank = INT32_MAX;

            std::size_t i = 0;
            while (i < colors.size())
            {
                int currentRank = std::abs(ColorToRank(colors[i]) - inColorRank);
                if (prevRank < currentRank)
                    break;
                prevRank = currentRank;
                ++i;
            }

            return std::max(0, static_cast<int>(i) - 1);
        }

        void Insert(LLGL::ColorRGBAub color)
        {
            colors.push_back(color);
        }

        // Finalizes the color palette by sorting them from brightest to darkest and opaque to transparent.
        void Finalize()
        {
            std::sort(
                colors.begin(),
                colors.end(),
                [](const LLGL::ColorRGBAub& lhs, const LLGL::ColorRGBAub& rhs) -> bool
                {
                    // First sort by opaque and transparent
                    if (lhs.a > rhs.a) { return true;  }
                    if (lhs.a < rhs.a) { return false; }

                    // Next sort by very crude brightness comparison. We don't care about luminance here, just use sum of all components
                    int lhsSum = lhs.r + lhs.g + lhs.b;
                    int rhsSum = rhs.r + rhs.g + rhs.b;

                    return lhsSum > rhsSum;
                }
            );

            // Find first transparent color
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                if (colors[i].a < 255)
                {
                    firstTransparentIndex = static_cast<int>(i);
                    break;
                }
            }
        }

        bool IsTransparent(int index) const
        {
            return (index >= firstTransparentIndex);
        }

        LLGL::ColorRGBAub IndexToColor(int index) const
        {
            return (index >= 0 && index < colors.size() ? colors[index] : LLGL::ColorRGBAub{});
        }

        std::uint32_t IndexToPacked(int index) const
        {
            return ColorToUint32(IndexToColor(index));
        }

        LLGL::ColorRGBAub operator [] (int index) const
        {
            return IndexToColor(index);
        }

        std::vector<LLGL::ColorRGBAub> GetColors() const
        {
            return colors;
        }

        int Count() const
        {
            return static_cast<int>(colors.size());
        }

        const void* Data() const
        {
            return colors.data();
        }

        std::size_t DataSize() const
        {
            return sizeof(LLGL::ColorRGBAub) * colors.size();
        }

        int HighlightColorIndex() const
        {
            return 0;
        }

    };

} // /namespace BrickUtils


#endif // /LLGL_EXAMPLE_BRICKPLAY_UTILS_H


