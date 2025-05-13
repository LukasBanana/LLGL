/*
 * PerlinNoise.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_PERLIN_NOISE_H
#define LLGLEXAMPLES_PERLIN_NOISE_H


#include <cstddef>
#include <cstdint>
#include <LLGL/STL/Vector.h>
#include <Gauss/Vector3.h>

using LLGL::STL::vector;

// Perlin noise generator class.
class PerlinNoise
{

    public:

        PerlinNoise();
        PerlinNoise(unsigned int seed);

        void Seed(unsigned int seed);

        // Returns a noise pattern in the range [0, 1].
        float Noise(float x, float y, float z, std::uint32_t frequency) const;
        float Noise(float x, float y, float z, std::uint32_t frequency, std::uint32_t octaves, float persistence = 0.5f) const;

        // Generates a perlin noise pattern into the output buffer.
        void GenerateBuffer(
            vector<float>&  buffer,
            std::uint32_t   width,
            std::uint32_t   height,
            std::uint32_t   depth,
            std::uint32_t   frequency,
            std::uint32_t   octaves     = 5,
            float           persistence = 0.5f
        );

        // Generates a perlin noise pattern into the output buffer.
        void GenerateBuffer(
            vector<std::uint8_t>&   buffer,
            std::uint32_t           width,
            std::uint32_t           height,
            std::uint32_t           depth,
            std::uint32_t           frequency,
            std::uint32_t           octaves     = 5,
            float                   persistence = 0.5f
        );

    private:

        void GeneratePermutations(std::uint32_t seed);
        void GenerateGradients();

    private:

        std::uint32_t   perm_[512];
        Gs::Vector3f    grads_[256];

};


#endif

