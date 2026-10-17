/*
 * BrickPlay.Utils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_UTILS_H
#define LLGL_EXAMPLE_BRICKPLAY_UTILS_H


#include "BrickPlay.Const.h"
#include <LLGL/Utils/ColorRGBA.h>
#include <LLGL/Platform/Platform.h>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <ctime> // to format std::chrono::time_point

#include <FileUtils.h> //TEST


// Formats std::time_point to string with strict C++11 code (no std::format).
template <typename TClock = std::chrono::system_clock>
inline std::string FormatTimePoint(const std::chrono::time_point<TClock>& timePoint, const char* format = "%Y-%m-%d_%H-%M-%S")
{
    // Convert time_point to std::time_t
    std::time_t time = TClock::to_time_t(timePoint);

    // Convert to local time structure
    std::tm tmBuf;
    #if _WIN32
    localtime_s(&tmBuf, &time);   // Thread-safe MSVC
    #else
    localtime_r(&time, &tmBuf);   // Thread-safe POSIX / GCC / Clang
    #endif

    // Format into C string
    char buffer[128] = {};
    if (std::strftime(buffer, sizeof(buffer), format, &tmBuf))
        return std::string(buffer);

    return "";
}

template <typename T, bool UseAcceleration = true>
T IntegrateForces(T& velocity, T& forces, float invMass, float dt, float dampingCoeff)
{
    if (UseAcceleration)
    {
        // Apply forces to velocity
        const T damping = -velocity * dampingCoeff;
        const T acceleration = (forces + damping) * invMass;
        velocity += acceleration * dt;
    }
    else
    {
        // Without physics integration, simply set velocity to input forces
        velocity = forces;
    }

    // Reset forces
    forces = {};

    return velocity * dt;
}

// Generic struct for simple physics integration. Used for camera movement and smooth menu bar scrolling.
template <typename T, bool UseAcceleration = true>
struct RigidBody
{
    T       forces      = {};
    T       velocity    = {};
    T       position    = {};
    float   invMass     = 1.0f;

    void SetMass(float mass)
    {
        invMass = 1.0f / mass;
    }

    void SetInfiniteMass()
    {
        invMass = 0.0f;
    }

    void AddForces(const T& inForces)
    {
        forces += inForces * invMass;
    }

    // Helper function for simple physics integration.
    T IntegrateForces(float dt, float dampingCoeff = 0.99f)
    {
        return ::IntegrateForces<T, UseAcceleration>(velocity, forces, invMass, dt, dampingCoeff);
    }
};

// Shortest signed rotation (in degrees) from `lhs` angle to `rhs` angle, in the range [-180, 180).
// Think of it as `rhs - lhs` but wrapping around, e.g. AngleDiff(350, 10) = 20 and AngleDiff(10, 350) = -20.
inline float AngleDiff(float lhs, float rhs)
{
    float diff = std::fmod(rhs - lhs + 180.0f, 360.0f);
    if (diff < 0.0f)
        diff += 360.0f;

    // Adding 360 to a tiny negative value can round up to exactly 360 in float precision
    if (diff >= 360.0f)
        diff = 0.0f;

    return diff - 180.0f;
}

struct AnimatedRotation
{
    float angle     = 0.0f;
    float velocity  = 0.0f; // Angular velocity
    float torque    = 0.0f; // Angular force

    float IntegrateForces(float invMass, float dt, float dampingCoeff = 0.99f)
    {
        return ::IntegrateForces<float>(velocity, torque, invMass, dt, dampingCoeff);
    }

    void AddTorque(float inTorque, float invMass)
    {
        torque += inTorque * invMass;
    }
};

// Free transformation, not bound by grid
class AnimatedTransform : RigidBody<Gs::Vector3f>
{
    enum class AnimState
    {
        Idle,
        LiftUp,
        WaitToRest,
        DelaySettleDown,
        SettleDown,
    };

    AnimState           yLiftAnimState  = AnimState::Idle;  // State of the animation to lift up and settle down the preview brick
    float               yLiftAnimPhase  = 0.0f;             // Animation phase from 0.0 (not lifted up) to 1.0 (fully lifted up)
    float               yLiftTimer      = 0.0f;             // Multi-purpose timer for the different animation states
    AnimatedRotation    yRotation;                          // Yaw
    float               xTilt           = 0.0f;             // Pitch
    float               zTilt           = 0.0f;             // Roll

    void ApplyForces(float dt, float dampingCoeff = 0.99f)
    {
        position += IntegrateForces(dt, dampingCoeff);
        yRotation.angle += yRotation.IntegrateForces(invMass, dt, dampingCoeff);
    }

    void AnimatePosition(const Gs::Vector3f& moveVec)
    {
        const float brickMoveCoefficient = 0.5f;
        AddForces(moveVec * brickMoveCoefficient);
    }

    void AnimateRotation(float yRotationSpin)
    {
        // Animate Y-rotation
        yRotation.AddTorque(yRotationSpin, invMass);
    }

    void AdvanceLiftUpAndSettleDownAnimation(float dt, const Gs::Vector3f& moveVec, float spin)
    {
        auto HasPositionOrRotationChanged = [](const Gs::Vector3f& vec, float angle) -> bool
        {
            constexpr float moveThreshold = 0.001f;
            constexpr float spinThreshold = 0.1f;
            return (vec.Length() > moveThreshold || std::abs(angle) > spinThreshold);
        };

        // When the target position or rotation changes, always reset the animation state to lifting up.
        const bool hasPositionOrRotationChanged = HasPositionOrRotationChanged(moveVec, spin);
        if (yLiftAnimState != AnimState::LiftUp && yLiftAnimState != AnimState::WaitToRest && hasPositionOrRotationChanged)
        {
            yLiftAnimState  = AnimState::LiftUp;
            yLiftTimer      = 0.0f;
        }

        switch (yLiftAnimState)
        {
            case AnimState::Idle:
                break;

            case AnimState::LiftUp:
                yLiftAnimPhase += dt / Cfg::animTimeToLiftUp;
                if (yLiftAnimPhase < 1.0f)
                    break;

                yLiftAnimPhase = 1.0f;
                yLiftAnimState = AnimState::WaitToRest;
                // [[fallthrough]]

            case AnimState::WaitToRest:
                // Advance animation of position and rotation
                AnimatePosition(moveVec);
                AnimateRotation(spin);

                // Once movement and rotation has stopped, delay the animation to settle the brick back down
                if (hasPositionOrRotationChanged || HasPositionOrRotationChanged(velocity, yRotation.velocity))
                    break;

                yLiftAnimState = AnimState::DelaySettleDown;
                // [[fallthrough]]

            case AnimState::DelaySettleDown:
                yLiftTimer += dt;
                if (yLiftTimer < Cfg::animDelayBeforeSettleDown)
                    break;

                yLiftAnimState = AnimState::SettleDown;
                // [[fallthrough]]

            case AnimState::SettleDown:
                yLiftAnimPhase -= dt / Cfg::animTimeToSettleDown;
                if (yLiftAnimPhase <= 0.0f)
                {
                    yLiftAnimPhase = 0.0f;
                    yLiftAnimState = AnimState::Idle;
                }
                break;
        }
    }

public:

    AnimatedTransform()
    {
        SetMass(0.025f);
    }

    float AdvanceAnimation(float dt, const Gs::Vector3f& targetPosition, float targetRotationY, bool isAnimRestricted)
    {
        const Gs::Vector3f moveVec = targetPosition - position;
        const float spin = AngleDiff(yRotation.angle, targetRotationY);

        if (isAnimRestricted)
        {
            // Advance animation of position and rotation
            AnimatePosition(moveVec);
            AnimateRotation(spin);

            // Reset restricted animation states
            yLiftAnimState = AnimState::Idle;
            yLiftAnimPhase = 0.0f;

            xTilt = 0.0f;
            zTilt = 0.0f;
        }
        else
        {
            // Advance the animation of lifting the brick up and down
            AdvanceLiftUpAndSettleDownAnimation(dt, moveVec, spin);

            auto MotionToTilt = [](float motion) -> float
            {
                float tiltAbs = Gs::Deg2Rad(std::logf(std::abs(motion) + 1.0f) * Cfg::animMotionTargetAngle);
                return (motion > 0.0f ? tiltAbs : -tiltAbs);
            };

            // Animate tilting the brick depending on motion
            xTilt = MotionToTilt(moveVec.x);
            zTilt = MotionToTilt(moveVec.z);
        }

        ApplyForces(dt);

        return Cfg::animLiftYAxis * yLiftAnimPhase;
    }

    const Gs::Vector3f& GetPosition() const
    {
        return position;
    }

    float GetXTilt() const
    {
        return xTilt;
    }

    float GetZTilt() const
    {
        return zTilt;
    }

    float GetYRotation() const
    {
        return Gs::Deg2Rad(yRotation.angle);
    }

    void PutAt(const Gs::Vector3f& inPosition, float inYRotation)
    {
        position        = inPosition;
        yRotation.angle = inYRotation;
    }
};

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
        static_cast<std::uint8_t>( col >> 24        ),
        static_cast<std::uint8_t>((col >> 16) & 0xFF),
        static_cast<std::uint8_t>((col >>  8) & 0xFF),
        static_cast<std::uint8_t>((col      ) & 0xFF)
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

        // Parse current line with stringstream (also trim trailing whitespaces)
        std::stringstream stream{ line.substr(0, line.find('#')) };

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

    static int ColorToRank(LLGL::ColorRGBAub col)
    {
        return col.a * 255*6 + (col.r*2 + col.g*3 + col.b);
    };

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
        const int inColorRank = ColorToRank(inColor);

        std::size_t lowestRankDiffIndex = 0;
        int lowestRankDiff = INT32_MAX;

        for_range(i, colors.size())
        {
            int rankDiff = std::abs(ColorToRank(colors[i]) - inColorRank);
            if (lowestRankDiff > rankDiff)
            {
                lowestRankDiffIndex = i;
                lowestRankDiff = rankDiff;
            }
        }

        return static_cast<int>(lowestRankDiffIndex);
    }

    void Insert(LLGL::ColorRGBAub color)
    {
        colors.push_back(color);
    }

    // Finalizes the color palette by sorting them from opaque to transparent.
    void Finalize()
    {
        std::sort(
            colors.begin(),
            colors.end(),
            [](const LLGL::ColorRGBAub& lhs, const LLGL::ColorRGBAub& rhs) -> bool
            {
                return (lhs.a > rhs.a);
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

};


#endif // /LLGL_EXAMPLE_BRICKPLAY_UTILS_H


