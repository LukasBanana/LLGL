/*
 * MTConstantsCacheLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_CONSTANTS_CACHE_LAYOUT_H
#define LLGL_MT_CONSTANTS_CACHE_LAYOUT_H


#import <Metal/Metal.h>

#include "../Shader/MTShaderStage.h"
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <cstdint>
#include <vector>


namespace LLGL
{


struct MTShaderReflectionArguments
{
    long                            stage;
    const NSArray<MTLArgument*>*    args;
};

// Stores the layout of shader constants data for uniforms.
class MTConstantsCacheLayout
{

    public:

        static constexpr std::uint16_t invalidOffset = 0xFFFF;

        struct ConstantLocation
        {
            std::uint16_t offsetPerStage[MTShaderStage_CountPerPSO] = { invalidOffset, invalidOffset };
            std::uint16_t size                                      = 0;
        };

        struct ConstantBuffer
        {
            long            stages : 16;
            NSUInteger      index  : 16;
            std::uint16_t   offset;
            std::uint16_t   size;
        };

    public:

        MTConstantsCacheLayout() = default;

        // Builds the internal descriptor lists for render and compute commands.
        MTConstantsCacheLayout(
            const ArrayView<MTShaderReflectionArguments>&   reflectionArgs,
            const ArrayView<UniformDescriptor>&             uniformDescs
        );

        // Returns the constants location map.
        inline const SmallVector<ConstantLocation, 8>& GetConstantsMap() const
        {
            return constantsMap_;
        }

        // Returns the constant buffer layouts.
        inline const SmallVector<ConstantBuffer, 2>& GetConstantBuffers() const
        {
            return constantBuffers_;
        }

        // Returns the required constants data cache size.
        inline std::uint16_t GetConstantsDataSize() const
        {
            return constantsDataSize_;
        }

    private:

        struct MTShaderBufferField
        {
            NSUInteger uniformIndex;    // Index to the uniform descriptor
            NSUInteger offset;          // Offset within the buffer
            NSUInteger size;            // Data type size

            static bool Equals(const MTShaderBufferField& lhs, const MTShaderBufferField& rhs);
        };

        struct MTShaderBuffer
        {
            ConstantBuffer                      cbuffer;
            std::vector<MTShaderBufferField>    fields;

            static bool EqualsFields(const MTShaderBuffer& lhs, const MTShaderBuffer& rhs);
        };

    private:

        static MTShaderBuffer* FindShaderBufferWithEqualField(
            std::vector<MTShaderBuffer>&    shaderBuffers,
            std::size_t                     compareBufferIndex
        );

        static MTShaderBuffer* FindOrAppendShaderBuffer(
            long                            stage,
            NSUInteger                      index,
            NSUInteger                      size,
            std::vector<MTShaderBuffer>&    shaderBuffers
        );

        static bool AppendUniformByName(
            const MTShaderReflectionArguments&  reflection,
            const UniformDescriptor&            uniformDesc,
            std::size_t                         uniformIndex,
            NSString*                           uniformName,
            std::vector<MTShaderBuffer>&        shaderBuffers
        );

        static void AppendUniformByDesc(
            const MTShaderReflectionArguments&  reflection,
            const UniformDescriptor&            uniformDesc,
            std::size_t                         uniformIndex,
            std::vector<MTShaderBuffer>&        shaderBuffers
        );

    private:

        std::uint16_t                       constantsDataSize_  = 0;
        SmallVector<ConstantLocation, 8>    constantsMap_;
        SmallVector<ConstantBuffer, 2>      constantBuffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
