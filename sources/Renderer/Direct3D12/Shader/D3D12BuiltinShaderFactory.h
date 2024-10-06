/*
 * D3D12BuiltinShaderFactory.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUILTIN_SHADER_FACTORY_H
#define LLGL_D3D12_BUILTIN_SHADER_FACTORY_H


#include "D3D12Shader.h"


namespace LLGL
{


class D3D12RootSignature;

// Enumeration of all builtin D3D11 shaders.
enum class D3D12BuiltinPSO
{
    StreamOutputDrawArgsCS = 0,
    Num
};

// Builtin D3D12 shader factory singleton.
class D3D12BuiltinShaderFactory
{

    public:

        D3D12BuiltinShaderFactory(const D3D12BuiltinShaderFactory&) = delete;
        D3D12BuiltinShaderFactory& operator = (const D3D12BuiltinShaderFactory&) = delete;

        // Returns the instance of this singleton.
        static D3D12BuiltinShaderFactory& Get();

        // Creats all builtin shaders with the specified D3D device.
        void CreateBuiltinPSOs(ID3D12Device* device);

        // Releases all builtin shaders.
        void Clear();

        // Returns the specified native builtin shader.
        bool GetBulitinPSO(const D3D12BuiltinPSO builtin, ID3D12PipelineState*& outPipelineState, ID3D12RootSignature*& outRootSignature) const;

    private:

        D3D12BuiltinShaderFactory() = default;

        void CreateComputePSO(
            ID3D12Device*           device,
            D3D12BuiltinPSO         builtin,
            D3D12RootSignature&     rootSignature,
            const unsigned char*    shaderBytecode,
            size_t                  shaderBytecodeSize
        );

    private:

        static constexpr std::size_t numBuiltinShaders = static_cast<std::size_t>(D3D12BuiltinPSO::Num);

        ComPtr<ID3D12RootSignature> rootSignatures_[D3D12BuiltinShaderFactory::numBuiltinShaders];
        ComPtr<ID3D12PipelineState> builtinPSOs_[D3D12BuiltinShaderFactory::numBuiltinShaders];

};


} // /namespace LLGL


#endif



// ================================================================================
