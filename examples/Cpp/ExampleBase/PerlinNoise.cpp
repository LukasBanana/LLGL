/*
 * PerlinNoise.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PerlinNoise.h"
#include <random>
#include <algorithm>
#include <limits.h>


PerlinNoise::PerlinNoise()
{
    GenerateGradients();
    Seed(0);
}

PerlinNoise::PerlinNoise(unsigned int seed)
{
    GenerateGradients();
    Seed(seed);
}

void PerlinNoise::Seed(unsigned int seed)
{
    GeneratePermutations(seed);
}

static int ModuloSignInt(int a, int b)
{
    return (((a % b + b)) % b);
}

float PerlinNoise::Noise(float x, float y, float z, std::uint32_t frequency) const
{
    frequency = std::max(1u, frequency);

    auto Fade = [](float t) -> float
    {
        return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
    };

    auto Surflet = [&](std::uint32_t xi, std::uint32_t yi, std::uint32_t zi)
    {
        const auto& p = perm_;
        auto hash = p[ p[ p[ ModuloSignInt(xi, frequency) ] + ModuloSignInt(yi, frequency) ] + ModuloSignInt(zi, frequency) ];

        auto grad =
        (
            (x - static_cast<float>(xi)) * grads_[hash].x +
            (y - static_cast<float>(yi)) * grads_[hash].y +
            (z - static_cast<float>(zi)) * grads_[hash].z
        );

        auto distX = std::abs(x - static_cast<float>(xi));
        auto distY = std::abs(y - static_cast<float>(yi));
        auto distZ = std::abs(z - static_cast<float>(zi));

        auto polyX = 1.0f - Fade(distX);
        auto polyY = 1.0f - Fade(distY);
        auto polyZ = 1.0f - Fade(distZ);

        return polyX * polyY * polyZ * grad;
    };

    auto xi = static_cast<std::uint32_t>(x);
    auto yi = static_cast<std::uint32_t>(y);
    auto zi = static_cast<std::uint32_t>(z);

    return
    (
        Surflet(xi, yi    , zi    ) + Surflet(xi + 1, yi    , zi    ) +
        Surflet(xi, yi + 1, zi    ) + Surflet(xi + 1, yi + 1, zi    ) +
        Surflet(xi, yi    , zi + 1) + Surflet(xi + 1, yi    , zi + 1) +
        Surflet(xi, yi + 1, zi + 1) + Surflet(xi + 1, yi + 1, zi + 1)
    );
}

float PerlinNoise::Noise(float x, float y, float z, std::uint32_t frequency, std::uint32_t octaves, float persistence) const
{
    frequency = std::max(1u, frequency);

    float noise     = 0.0f;
    float amplitude = 1.0f;

    for (std::uint32_t i = 0; i < octaves; ++i)
    {
        noise       += Noise(x, y, z, frequency) * amplitude;
        x           *= 2.0f;
        y           *= 2.0f;
        z           *= 2.0f;
        amplitude   *= persistence;
        frequency   *= 2;
    }

    return (noise * 0.5f + 0.5f);
}

void PerlinNoise::GenerateBuffer(
    std::vector<float>& buffer,
    std::uint32_t       width,
    std::uint32_t       height,
    std::uint32_t       depth,
    std::uint32_t       frequency,
    std::uint32_t       octaves,
    float               persistence)
{
    buffer.resize(static_cast<std::size_t>(width*height*depth));

    auto invWidth   = static_cast<float>(frequency) / static_cast<float>(width);
    auto invHeight  = static_cast<float>(frequency) / static_cast<float>(height);
    auto invDepth   = static_cast<float>(frequency) / static_cast<float>(depth);

    std::size_t i = 0;

    for (std::uint32_t z = 0; z < depth; ++z)
    {
        for (std::uint32_t y = 0; y < height; ++y)
        {
            for (std::uint32_t x = 0; x < width; ++x)
            {
                buffer[i] = Noise(
                    static_cast<float>(x) * invWidth,
                    static_cast<float>(y) * invHeight,
                    static_cast<float>(z) * invDepth,
                    frequency,
                    octaves,
                    persistence
                );
                ++i;
            }
        }
    }
}

void PerlinNoise::GenerateBuffer(
    std::vector<std::uint8_t>&  buffer,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               depth,
    std::uint32_t               frequency,
    std::uint32_t               octaves,
    float                       persistence)
{
    buffer.resize(static_cast<std::size_t>(width*height*depth));

    auto invWidth   = static_cast<float>(frequency) / static_cast<float>(width);
    auto invHeight  = static_cast<float>(frequency) / static_cast<float>(height);
    auto invDepth   = static_cast<float>(frequency) / static_cast<float>(depth);

    auto maxValue   = static_cast<float>(UCHAR_MAX);

    std::size_t i = 0;

    for (std::uint32_t z = 0; z < depth; ++z)
    {
        for (std::uint32_t y = 0; y < height; ++y)
        {
            for (std::uint32_t x = 0; x < width; ++x)
            {
                auto noise = Noise(
                    static_cast<float>(x) * invWidth,
                    static_cast<float>(y) * invHeight,
                    static_cast<float>(z) * invDepth,
                    frequency,
                    octaves,
                    persistence
                );
                buffer[i] = static_cast<std::uint8_t>(noise * maxValue);
                ++i;
            }
        }
    }
}


/*
 * ======= Private: =======
 */

void PerlinNoise::GeneratePermutations(std::uint32_t seed)
{
    static const int n = 256;

    for (int i = 0; i < n; ++i)
        perm_[i] = i;

    std::shuffle(std::begin(perm_), std::begin(perm_) + n, std::default_random_engine(seed));

    for (int i = 0; i < n; ++i)
        perm_[n + i] = perm_[i];
}

void PerlinNoise::GenerateGradients()
{
    auto angleStep = Gs::pi * 2.0f / static_cast<float>(grads_.size());

    for (std::size_t i = 0; i < grads_.size(); ++i)
    {
        auto a = static_cast<float>(i) * angleStep;
        grads_[i] = Gs::Vector3f{ std::cos(a), std::sin(a), std::sin(a)*std::cos(a) };
    }
}

