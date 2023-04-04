/*
 * MTConstantsCache.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_CONSTANTS_CACHE_H
#define LLGL_MT_CONSTANTS_CACHE_H


#import <Metal/Metal.h>

#include "../Shader/MTShaderStage.h"
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <memory>
#include <cstdint>
#include <vector>


namespace LLGL
{


class MTShader;

struct MTShaderReflectionArguments
{
    long                            stage;
    const NSArray<MTLArgument*>*    args;
};

// Manages the shader constants data for uniforms. Maximum size of such a cache is 4KB (as per Metal spec.).
class MTConstantsCache
{

    public:

        MTConstantsCache() = default;

        // Builds the internal descriptor lists for render and compute commands.
        MTConstantsCache(
            const ArrayView<MTShaderReflectionArguments>&   reflectionArgs,
            const ArrayView<UniformDescriptor>&             uniformDescs
        );

        // Resets the dirty bits which will bind all resources on the next flush, i.e. IsInvalidated() returns true.
        void Reset();

        // Sets the specified resource in this cache.
        void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize);

        // Flushes the pending descriptors to the specified command encoder.
        void FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder);
        void FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder);
        void FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder);

        // Returns true if this cache has been invalidated.
        inline bool IsInvalidated() const
        {
            return (dirtyBits_.bits != 0);
        }

    private:

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

      //SmallVector<ConstantLocation, 8>    constantsMap_;
      //SmallVector<ConstantBuffer, 2>      constantBuffers_;
        std::vector<ConstantLocation>       constantsMap_;
        std::vector<ConstantBuffer>         constantBuffers_;
        std::unique_ptr<char[]>             constants_;

        union
        {
            std::uint8_t bits;
            struct
            {
                std::uint8_t graphics : 1;
                std::uint8_t compute  : 1;
            };
        }
        dirtyBits_;

};


} // /namespace LLGL


#endif



// ================================================================================
