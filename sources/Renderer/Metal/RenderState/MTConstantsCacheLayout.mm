/*
 * MTConstantsCacheLayout.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTConstantsCacheLayout.h"
#include "../Shader/MTShader.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <string.h>


namespace LLGL
{


MTConstantsCacheLayout::MTConstantsCacheLayout(
    const ArrayView<MTShaderReflectionArguments>&   reflectionArgs,
    const ArrayView<UniformDescriptor>&             uniformDescs)
{
    /* Find structure field in Metal shader reflection for each uniform per reflection */
    std::vector<MTShaderBuffer> shaderBuffers;
    for (const MTShaderReflectionArguments& reflection : reflectionArgs)
    {
        for_range(i, uniformDescs.size())
            AppendUniformByDesc(reflection, uniformDescs[i], i, shaderBuffers);
    }

    /* Try to consolidate all uniforms that share the same buffer index and struct offsets */
    constantsDataSize_ = (!shaderBuffers.empty() ? shaderBuffers.front().cbuffer.size : 0);
    for_subrange(i, 1u, shaderBuffers.size())
    {
        MTShaderBuffer& dstBuffer = shaderBuffers[i];
        if (MTShaderBuffer* sharedShaderBuffer = FindShaderBufferWithEqualField(shaderBuffers, i))
        {
            /* Merge stages into shared buffer */
            sharedShaderBuffer->cbuffer.stages |= dstBuffer.cbuffer.stages;
            dstBuffer.cbuffer.stages = 0;

            /* Don't allocate dedicated memory for this buffer since we share it with the merged buffer */
            dstBuffer.fields.clear();
            dstBuffer.cbuffer.offset = sharedShaderBuffer->cbuffer.offset;
        }
        else
        {
            /* Allocate dedicate memory for this buffer */
            dstBuffer.cbuffer.offset = constantsDataSize_;
            constantsDataSize_ += dstBuffer.cbuffer.size;
        }
    }

    /* Copy constant buffer ranges and fill constants map */
    constantsMap_.resize(uniformDescs.size());

    for_range(i, shaderBuffers.size())
    {
        const ConstantBuffer& cbuffer = shaderBuffers[i].cbuffer;
        if (cbuffer.stages != 0)
        {
            constantBuffers_.push_back(cbuffer);
            for (const MTShaderBufferField& field : shaderBuffers[i].fields)
            {
                ConstantLocation& mapping = constantsMap_[field.uniformIndex];
                {
                    const std::uint16_t constantOffset = cbuffer.offset + static_cast<std::uint16_t>(field.offset);
                    if (mapping.offsetPerStage[0] == MTConstantsCacheLayout::invalidOffset)
                        mapping.offsetPerStage[0] = constantOffset;
                    else
                        mapping.offsetPerStage[1] = constantOffset;
                }
                mapping.size = static_cast<std::uint16_t>(field.size);
            }
        }
    }
}


/*
 * ======= Private: =======
 */

MTConstantsCacheLayout::MTShaderBuffer* MTConstantsCacheLayout::FindShaderBufferWithEqualField(
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

MTConstantsCacheLayout::MTShaderBuffer* MTConstantsCacheLayout::FindOrAppendShaderBuffer(
    long                            stage,
    NSUInteger                      index,
    NSUInteger                      size,
    std::vector<MTShaderBuffer>&    shaderBuffers)
{
    std::size_t insertionPos = 0;
    MTShaderBuffer* dstBuffer = FindInSortedArray<MTShaderBuffer>(
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

bool MTConstantsCacheLayout::AppendUniformByName(
    const MTShaderReflectionArguments&  reflection,
    const UniformDescriptor&            uniformDesc,
    std::size_t                         uniformIndex,
    NSString*                           uniformName,
    std::vector<MTShaderBuffer>&        shaderBuffers)
{
    for (MTLArgument* arg in reflection.args)
    {
        if (arg.type == MTLArgumentTypeBuffer)
        {
            if (MTLStructType* structType = arg.bufferStructType)
            {
                if (MTLStructMember* member = [structType memberByName:uniformName])
                {
                    MTShaderBuffer* dstBuffer = FindOrAppendShaderBuffer(reflection.stage, arg.index, arg.bufferDataSize, shaderBuffers);
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
    }
    return false;
}

void MTConstantsCacheLayout::AppendUniformByDesc(
    const MTShaderReflectionArguments&  reflection,
    const UniformDescriptor&            uniformDesc,
    std::size_t                         uniformIndex,
    std::vector<MTShaderBuffer>&        shaderBuffers)
{
    NSString* uniformName = [NSString stringWithCString:uniformDesc.name.c_str() encoding:NSASCIIStringEncoding];
    AppendUniformByName(reflection, uniformDesc, uniformIndex, uniformName, shaderBuffers);
}


/*
 * MTShaderBufferField structure
 */

bool MTConstantsCacheLayout::MTShaderBufferField::Equals(const MTShaderBufferField& lhs, const MTShaderBufferField& rhs)
{
    return (lhs.uniformIndex == rhs.uniformIndex && lhs.offset == rhs.offset && lhs.size == rhs.size);
}


/*
 * MTShaderBuffer structure
 */

bool MTConstantsCacheLayout::MTShaderBuffer::EqualsFields(const MTShaderBuffer& lhs, const MTShaderBuffer& rhs)
{
    if (lhs.fields.size() == rhs.fields.size())
    {
        for_range(i, lhs.fields.size())
        {
            if (!MTConstantsCacheLayout::MTShaderBufferField::Equals(lhs.fields[i], rhs.fields[i]))
                return false;
        }
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
