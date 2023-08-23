/*
 * MTBuiltinPSOFactory.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTBuiltinPSOFactory.h"
#include "../Shader/MTShader.h"
#include "../Shader/Builtin/MTBuiltin.h"
#include "../MTCore.h"
#include "../../../Core/Exception.h"
#include <LLGL/Report.h>


namespace LLGL
{


MTBuiltinPSOFactory& MTBuiltinPSOFactory::Get()
{
    static MTBuiltinPSOFactory instance;
    return instance;
}

void MTBuiltinPSOFactory::CreateBuiltinPSOs(id<MTLDevice> device)
{
    LoadBuiltinComputePSO(device, MTBuiltinComputePSO::FillBufferByte4, g_metalLibFillBufferByte4, g_metalLibFillBufferByte4Len);
}

id<MTLComputePipelineState> MTBuiltinPSOFactory::GetComputePSO(const MTBuiltinComputePSO builtin) const
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

void MTBuiltinPSOFactory::LoadBuiltinComputePSO(
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

    /* We cannot recover from a faulty built-in shader */
    if (const Report* report = cs.GetReport())
    {
        if (report->HasErrors())
            LLGL_TRAP("%s", report->GetText());
    }

    /* Create native compute pipeline state */
    const std::size_t idx = static_cast<std::size_t>(builtin);
    NSError* error = nullptr;
    builtinComputePSOs_[idx] = [device newComputePipelineStateWithFunction:cs.GetNative() error:&error];
    if (!builtinComputePSOs_[idx])
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}


} // /namespace LLGL



// ================================================================================
