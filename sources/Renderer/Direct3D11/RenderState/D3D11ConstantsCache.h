/*
 * D3D11ConstantsCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_CONSTANTS_CACHE_H
#define LLGL_D3D11_CONSTANTS_CACHE_H


#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "../Shader/D3D11Shader.h"
#include <cstdint>
#include <vector>
#include <d3d11.h>
#include <memory>


namespace LLGL
{


class D3D11Shader;
class D3D11StateManager;

// Manages the CPU data of a D3D11 constant buffer for dynamic uniforms; See 'PipelineLayoutDescriptor::uniforms'.
class D3D11ConstantsCache
{

    public:

        D3D11ConstantsCache(const D3D11ConstantsCache&) = delete;
        D3D11ConstantsCache& operator = (const D3D11ConstantsCache&) = delete;

        D3D11ConstantsCache(
            const ArrayView<D3D11Shader*>&      shaders,
            const ArrayView<UniformDescriptor>& uniforms
        );

        HRESULT SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize);

        // Resets the internal cache to bind all constants again at the next call to Flush().
        void Reset();

        // Sets the constant buffer values for the cbuffers that have changed.
        void Flush(D3D11StateManager& stateMngr);

    private:

        // Uniform to cbuffer field mapping structure.
        struct ConstantLocation
        {
            std::uint8_t    index;  // Constant buffer index into 'constantBuffers_'
            std::uint8_t    size;
            std::uint16_t   offset;
        };

        // Represents a single D3D constant register of four 32-bit words.
        struct alignas(16) ConstantRegister
        {
            std::uint32_t words[4];
        };

        struct ConstantBuffer
        {
            UINT                            shaderRegister; // Constant buffer binding slot
            long                            stageFlags;
            std::vector<ConstantRegister>   constants;
        };

    private:

        // Allocates a new constant buffer at the specified slot and returns its zero-based index.
        std::uint8_t AllocateConstantBuffer(UINT slot, UINT size, long stageFlags);

        // Sets the constants of the specified cbuffer with the state manager.
        void FlushConstantBuffer(std::uint8_t index, D3D11StateManager& stateMngr);

    private:

        SmallVector<ConstantLocation, 8>    constantsMap_;
        SmallVector<ConstantBuffer, 2>      constantBuffers_;
        SmallVector<bool, 2>                invalidatedBuffers_;
        std::uint8_t                        invalidatedBuffersRange_[2] = { 0xFF, 0x00 };

};

using D3D11ConstantsCachePtr = std::unique_ptr<D3D11ConstantsCache>;


} // /namespace LLGL


#endif



// ================================================================================
