/*
 * MTBuiltinShaderPool.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTBuiltinShaderPool.h"
#include "MTShader.h"
#include "Builtin/MTBuiltin.h"
#include "../MTCore.h"


namespace LLGL
{


MTBuiltinShaderPool& MTBuiltinShaderPool::Get()
{
    static MTBuiltinShaderPool g_instance;
    return g_instance;
}

void MTBuiltinShaderPool::LoadBuiltinShaders(id<MTLDevice> device)
{
    LoadBuiltinComputePSO(device, MTBuiltinComputePSO::FillBufferByte4, g_metalLibFillBufferByte4, g_metalLibFillBufferByte4Len);
}

id<MTLComputePipelineState> MTBuiltinShaderPool::GetComputePSO(const MTBuiltinComputePSO builtin) const
{
    const auto idx = static_cast<std::size_t>(builtin);
    if (idx < g_numComputePSOs)
        return builtinComputePSOs_[idx];
    else
        return nil;
}


/*
 * ======= Private: =======
 */

void MTBuiltinShaderPool::LoadBuiltinComputePSO(
    id<MTLDevice>               device,
    const MTBuiltinComputePSO   builtin,
    const char*                 kernelFunc,
    std::size_t                 kernelFuncSize)
{
    /* Load compute shader function */
    ShaderDescriptor shaderDesc;
    {
        shaderDesc.type         = ShaderType::Compute;
        shaderDesc.source       = kernelFunc;
        shaderDesc.sourceSize   = kernelFuncSize;
        shaderDesc.sourceType   = ShaderSourceType::BinaryBuffer;
        shaderDesc.entryPoint   = "CS";
        shaderDesc.profile      = "1.1";
    }
    MTShader cs{ device, shaderDesc };

    /* Create native compute pipeline state */
    const auto idx = static_cast<std::size_t>(builtin);
    NSError* error = nullptr;
    builtinComputePSOs_[idx] = [device newComputePipelineStateWithFunction:cs.GetNative() error:&error];
    if (!builtinComputePSOs_[idx])
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}


} // /namespace LLGL



// ================================================================================
