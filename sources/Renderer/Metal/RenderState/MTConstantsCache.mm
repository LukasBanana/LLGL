/*
 * MTConstantsCache.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTConstantsCache.h"
#include "../Shader/MTShader.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <string.h>


namespace LLGL
{


MTConstantsCache::MTConstantsCache(
    const ArrayView<MTShaderReflectionArguments>&   reflectionArgs,
    const ArrayView<UniformDescriptor>&             uniformDescs)
{
    /* Find structure field in Metal shader reflection for each uniform per reflection */
    std::vector<MTShaderBuffer> shaderBuffers;
    for (const auto& reflection : reflectionArgs)
    {
        for_range(i, uniformDescs.size())
            AppendUniformByDesc(reflection, uniformDescs[i], i, shaderBuffers);
    }

    /* Try to consolidate all uniforms that share the same buffer index and struct offsets */
    std::uint16_t constantsDataSize = (!shaderBuffers.empty() ? shaderBuffers.front().cbuffer.size : 0);
    for_subrange(i, 1u, shaderBuffers.size())
    {
        auto& dstBuffer = shaderBuffers[i];
        if (auto* sharedBuffer = FindShaderBufferWithEqualField(shaderBuffers, i))
        {
            /* Merge stages into shared buffer */
            sharedBuffer->cbuffer.stages |= dstBuffer.cbuffer.stages;
            dstBuffer.cbuffer.stages = 0;

            /* Don't allocate dedicated memory for this buffer since we share it with the merged buffer */
            dstBuffer.fields.clear();
            dstBuffer.cbuffer.offset = sharedBuffer->cbuffer.offset;
        }
        else
        {
            /* Allocate dedicate memory for this buffer */
            dstBuffer.cbuffer.offset = constantsDataSize;
            constantsDataSize += dstBuffer.cbuffer.size;
        }
    }

    /* Allocate and initialize constants memory */
    constants_ = MakeUniqueArray<char>(constantsDataSize);
    #ifdef LLGL_DEBUG
    ::memset(constants_.get(), 0xDEADBEEF, constantsDataSize);
    #endif

    /* Copy constant buffer ranges and fill constants map */
    constantsMap_.resize(uniformDescs.size());

    for_range(i, shaderBuffers.size())
    {
        const auto& cbuffer = shaderBuffers[i].cbuffer;
        if (cbuffer.stages != 0)
        {
            constantBuffers_.push_back(cbuffer);
            for (const auto& field : shaderBuffers[i].fields)
            {
                auto& mapping = constantsMap_[field.uniformIndex];
                {
                    const std::uint16_t constantOffset = cbuffer.offset + static_cast<std::uint16_t>(field.offset);
                    if (mapping.offsetPerStage[0] == MTConstantsCache::invalidOffset)
                        mapping.offsetPerStage[0] = constantOffset;
                    else
                        mapping.offsetPerStage[1] = constantOffset;
                }
                mapping.size = static_cast<std::uint16_t>(field.size);
            }
        }
    }

    Reset();
}

void MTConstantsCache::Reset()
{
    dirtyBits_.bits = 0xFF;
}

void MTConstantsCache::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    for (auto* bytes = reinterpret_cast<const char*>(data); first < constantsMap_.size() && dataSize > 0; ++first)
    {
        /* Early exit even ignoring the dirty bits since out-of-bounds is undefined behavior */
        const auto& constant = constantsMap_[first];
        if (constant.size > dataSize)
            return /*Out of bounds*/;

        /* Update constant data for each shader stage */
        for_range(i, MTShaderStage_CountPerPSO)
        {
            if (constant.offsetPerStage[i] != MTConstantsCache::invalidOffset)
                ::memcpy(constants_.get() + constant.offsetPerStage[i], bytes, constant.size);
        }

        /* Move to next uniform */
        dataSize -= constant.size;
        bytes += constant.size;
    }
    Reset();
}

void MTConstantsCache::FlushGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder)
{
    if (dirtyBits_.graphics != 0)
        FlushGraphicsResourcesForced(renderEncoder);
}

void MTConstantsCache::FlushGraphicsResourcesForced(id<MTLRenderCommandEncoder> renderEncoder)
{
    for (const auto& constantBuffer : constantBuffers_)
    {
        if ((constantBuffer.stages & StageFlags::VertexStage) != 0)
        {
            [renderEncoder
                setVertexBytes: constants_.get() + constantBuffer.offset
                length:         constantBuffer.size
                atIndex:        constantBuffer.index
            ];
        }
        if ((constantBuffer.stages & StageFlags::FragmentStage) != 0)
        {
            [renderEncoder
                setFragmentBytes:   constants_.get() + constantBuffer.offset
                length:             constantBuffer.size
                atIndex:            constantBuffer.index
            ];
        }
    }
    dirtyBits_.graphics = 0;
}

void MTConstantsCache::FlushComputeResources(id<MTLComputeCommandEncoder> computeEncoder)
{
    if (dirtyBits_.compute != 0)
        FlushComputeResourcesForced(computeEncoder);
}

void MTConstantsCache::FlushComputeResourcesForced(id<MTLComputeCommandEncoder> computeEncoder)
{
    for (const auto& constantBuffer : constantBuffers_)
    {
        if ((constantBuffer.stages & StageFlags::ComputeStage) != 0)
        {
            [computeEncoder
                setBytes:   constants_.get() + constantBuffer.offset
                length:     constantBuffer.size
                atIndex:    constantBuffer.index
            ];
        }
    }
    dirtyBits_.compute = 0;
}


/*
 * ======= Private: =======
 */

MTConstantsCache::MTShaderBuffer* MTConstantsCache::FindShaderBufferWithEqualField(
    std::vector<MTShaderBuffer>&    shaderBuffers,
    std::size_t                     compareBufferIndex)
{
    for_range(i, compareBufferIndex)
    {
        if (MTShaderBuffer::EqualsFields(shaderBuffers[i], shaderBuffers[compareBufferIndex]))
            return &(shaderBuffers[i]);
    }
    return nullptr;
}

MTConstantsCache::MTShaderBuffer* MTConstantsCache::FindOrAppendShaderBuffer(
    long                            stage,
    NSUInteger                      index,
    NSUInteger                      size,
    std::vector<MTShaderBuffer>&    shaderBuffers)
{
    std::size_t insertionPos = 0;
    auto* dstBuffer = FindInSortedArray<MTShaderBuffer>(
        shaderBuffers.data(),
        shaderBuffers.size(),
        [stage, index](const MTShaderBuffer& entry) -> int
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(entry.cbuffer.stages, stage);
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(entry.cbuffer.index, index);
            return 0;
        },
        &insertionPos
    );
    if (dstBuffer == nullptr)
    {
        MTShaderBuffer newBuffer;
        {
            newBuffer.cbuffer.stages    = stage;
            newBuffer.cbuffer.index     = index;
            newBuffer.cbuffer.offset    = 0;
            newBuffer.cbuffer.size      = static_cast<std::uint16_t>(size);
        }
        shaderBuffers.insert(shaderBuffers.begin() + insertionPos, newBuffer);
        return &(shaderBuffers[insertionPos]);
    }
    return dstBuffer;
}

bool MTConstantsCache::AppendUniformByName(
    const MTShaderReflectionArguments&  reflection,
    const UniformDescriptor&            uniformDesc,
    std::size_t                         uniformIndex,
    NSString*                           uniformName,
    std::vector<MTShaderBuffer>&        shaderBuffers)
{
    for (MTLArgument* arg in reflection.args)
    {
        if (MTLStructType* structType = arg.bufferStructType)
        {
            if (MTLStructMember* member = [structType memberByName:uniformName])
            {
                auto* dstBuffer = FindOrAppendShaderBuffer(reflection.stage, arg.index, arg.bufferDataSize, shaderBuffers);
                MTShaderBufferField field;
                {
                    field.uniformIndex  = static_cast<NSUInteger>(uniformIndex);
                    field.offset        = member.offset;
                    field.size          = GetUniformTypeSize(uniformDesc.type, uniformDesc.arraySize);
                }
                dstBuffer->fields.push_back(field);
                return true;
            }
        }
    }
    return false;
}

void MTConstantsCache::AppendUniformByDesc(
    const MTShaderReflectionArguments&  reflection,
    const UniformDescriptor&            uniformDesc,
    std::size_t                         uniformIndex,
    std::vector<MTShaderBuffer>&        shaderBuffers)
{
    NSString* uniformName = [NSString stringWithCString:uniformDesc.name.c_str() encoding:NSASCIIStringEncoding];
    AppendUniformByName(reflection, uniformDesc, uniformIndex, uniformName, shaderBuffers);
    [uniformName release];
}


/*
 * MTShaderBufferField structure
 */

bool MTConstantsCache::MTShaderBufferField::Equals(const MTShaderBufferField& lhs, const MTShaderBufferField& rhs)
{
    return (lhs.uniformIndex == rhs.uniformIndex && lhs.offset == rhs.offset && lhs.size == rhs.size);
}


/*
 * MTShaderBuffer structure
 */

bool MTConstantsCache::MTShaderBuffer::EqualsFields(const MTShaderBuffer& lhs, const MTShaderBuffer& rhs)
{
    if (lhs.fields.size() == rhs.fields.size())
    {
        for_range(i, lhs.fields.size())
        {
            if (!MTConstantsCache::MTShaderBufferField::Equals(lhs.fields[i], rhs.fields[i]))
                return false;
        }
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
