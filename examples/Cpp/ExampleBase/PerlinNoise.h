/*
 * PerlinNoise.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PERLIN_NOISE_H
#define LLGL_PERLIN_NOISE_H


#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>
#include <Gauss/Vector3.h>


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
            std::vector<float>& buffer,
            std::uint32_t       width,
            std::uint32_t       height,
            std::uint32_t       depth,
            std::uint32_t       frequency,
            std::uint32_t       octaves     = 5,
            float               persistence = 0.5f
        );

        // Generates a perlin noise pattern into the output buffer.
        void GenerateBuffer(
            std::vector<std::uint8_t>&  buffer,
            std::uint32_t               width,
            std::uint32_t               height,
            std::uint32_t               depth,
            std::uint32_t               frequency,
            std::uint32_t               octaves     = 5,
            float                       persistence = 0.5f
        );

    private:

        void GeneratePermutations(std::uint32_t seed);
        void GenerateGradients();

    private:

        std::array<std::uint32_t, 512>  perm_;
        std::array<Gs::Vector3f, 256>   grads_;

};


#endif

