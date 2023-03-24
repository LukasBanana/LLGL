/*
 * PipelineStateUtils.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PipelineStateUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static bool HasAnyStencilOpReplace(const StencilFaceDescriptor& desc)
{
    return
    (
        desc.stencilFailOp == StencilOp::Replace ||
        desc.depthFailOp   == StencilOp::Replace ||
        desc.depthPassOp   == StencilOp::Replace
    );
}

static bool HasAnyStencilOpReplace(const StencilDescriptor& desc)
{
    return
    (
        HasAnyStencilOpReplace(desc.front) ||
        HasAnyStencilOpReplace(desc.back)
    );
}

LLGL_EXPORT bool IsStencilRefEnabled(const StencilDescriptor& desc)
{
    return (desc.testEnabled && HasAnyStencilOpReplace(desc));
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


} // /namespace LLGL



// ================================================================================
