/*
 * PipelineStateUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "PipelineStateUtils.h"
#include "../Core/CoreUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static bool HasAnyStencilRefWrite(const StencilFaceDescriptor& desc)
{
    return
    (
        desc.writeMask != 0 &&
        (
            desc.stencilFailOp == StencilOp::Replace ||
            desc.depthFailOp   == StencilOp::Replace ||
            desc.depthPassOp   == StencilOp::Replace
        )
    );
}

static bool HasAnyStencilRefRead(const StencilFaceDescriptor& desc)
{
    return (desc.readMask != 0 && desc.compareOp != CompareOp::NeverPass && desc.compareOp != CompareOp::AlwaysPass);
}

static bool HasAnyStencilRefUse(const StencilDescriptor& desc)
{
    return
    (
        HasAnyStencilRefWrite(desc.front) ||
        HasAnyStencilRefWrite(desc.back)  ||
        HasAnyStencilRefRead(desc.front)  ||
        HasAnyStencilRefRead(desc.back)
    );
}

LLGL_EXPORT bool IsStencilRefEnabled(const StencilDescriptor& desc)
{
    return (desc.testEnabled && HasAnyStencilRefUse(desc));
}

LLGL_EXPORT bool IsStaticStencilRefEnabled(const StencilDescriptor& desc)
{
    return (!desc.referenceDynamic && IsStencilRefEnabled(desc));
}

static bool IsBlendOpUsingBlendFactor(const BlendOp op)
{
    return (op == BlendOp::BlendFactor || op == BlendOp::InvBlendFactor);
}

static bool IsTargetUsingBlendFactor(const BlendTargetDescriptor& desc)
{
    return
    (
        desc.blendEnabled &&
        (
            IsBlendOpUsingBlendFactor(desc.srcColor) ||
            IsBlendOpUsingBlendFactor(desc.dstColor) ||
            IsBlendOpUsingBlendFactor(desc.srcAlpha) ||
            IsBlendOpUsingBlendFactor(desc.dstAlpha)
        )
    );
}

LLGL_EXPORT bool IsBlendFactorEnabled(const BlendDescriptor& desc)
{
    if (desc.independentBlendEnabled)
    {
        for (const auto& target : desc.targets)
        {
            if (IsTargetUsingBlendFactor(target))
                return true;
        }
    }
    else
    {
        if (IsTargetUsingBlendFactor(desc.targets[0]))
            return true;
    }
    return false;
}

LLGL_EXPORT bool IsStaticBlendFactorEnabled(const BlendDescriptor& desc)
{
    return (!desc.blendFactorDynamic && IsBlendFactorEnabled(desc));
}

template <std::size_t N>
inline void AddShaderIfSet(SmallVector<Shader*, N>& shaders, Shader* sh)
{
    if (sh != nullptr)
        shaders.push_back(sh);
};

LLGL_EXPORT SmallVector<Shader*, 5> GetShadersAsArray(const GraphicsPipelineDescriptor& desc)
{
    SmallVector<Shader*, 5> shaders;
    AddShaderIfSet(shaders, desc.vertexShader);
    AddShaderIfSet(shaders, desc.tessControlShader);
    AddShaderIfSet(shaders, desc.tessEvaluationShader);
    AddShaderIfSet(shaders, desc.geometryShader);
    AddShaderIfSet(shaders, desc.fragmentShader);
    return shaders;
}

LLGL_EXPORT SmallVector<Shader*, 1> GetShadersAsArray(const ComputePipelineDescriptor& desc)
{
    SmallVector<Shader*, 1> shaders;
    AddShaderIfSet(shaders, desc.computeShader);
    return shaders;
}

static std::uint32_t GetUniformBaseTypeSize(UniformType type)
{
    switch (type)
    {
        case UniformType::Undefined:        break;

        case UniformType::Float1:           return 4;
        case UniformType::Float2:           return 4*2;
        case UniformType::Float3:           return 4*3;
        case UniformType::Float4:           return 4*4;
        case UniformType::Double1:          return 8;
        case UniformType::Double2:          return 8*2;
        case UniformType::Double3:          return 8*3;
        case UniformType::Double4:          return 8*4;
        case UniformType::Int1:             return 4;
        case UniformType::Int2:             return 4*2;
        case UniformType::Int3:             return 4*3;
        case UniformType::Int4:             return 4*4;
        case UniformType::UInt1:            return 4;
        case UniformType::UInt2:            return 4*2;
        case UniformType::UInt3:            return 4*3;
        case UniformType::UInt4:            return 4*4;
        case UniformType::Bool1:            return 4;
        case UniformType::Bool2:            return 4*2;
        case UniformType::Bool3:            return 4*3;
        case UniformType::Bool4:            return 4*4;
        case UniformType::Float2x2:         return 4*2*2;
        case UniformType::Float2x3:         return 4*2*3;
        case UniformType::Float2x4:         return 4*2*4;
        case UniformType::Float3x2:         return 4*3*2;
        case UniformType::Float3x3:         return 4*3*3;
        case UniformType::Float3x4:         return 4*3*4;
        case UniformType::Float4x2:         return 4*4*2;
        case UniformType::Float4x3:         return 4*4*3;
        case UniformType::Float4x4:         return 4*4*4;
        case UniformType::Double2x2:        return 8*2*2;
        case UniformType::Double2x3:        return 8*2*3;
        case UniformType::Double2x4:        return 8*2*4;
        case UniformType::Double3x2:        return 8*3*2;
        case UniformType::Double3x3:        return 8*3*3;
        case UniformType::Double3x4:        return 8*3*4;
        case UniformType::Double4x2:        return 8*4*2;
        case UniformType::Double4x3:        return 8*4*3;
        case UniformType::Double4x4:        return 8*4*4;

        case UniformType::Sampler:          break;
        case UniformType::Image:            break;
        case UniformType::AtomicCounter:    break;
    }
    return 0;
}

// Returns the size (in bytes) of the specified shader uniform type.
LLGL_EXPORT std::uint32_t GetUniformTypeSize(UniformType type, std::uint32_t arraySize)
{
    if (arraySize > 1)
    {
        const auto typeSizeVec4Aligned = GetAlignedSize(GetUniformBaseTypeSize(type), GetUniformBaseTypeSize(UniformType::Float4));
        return (typeSizeVec4Aligned * (arraySize - 1) + GetUniformBaseTypeSize(type));
    }
    return GetUniformBaseTypeSize(type);
}


} // /namespace LLGL



// ================================================================================
